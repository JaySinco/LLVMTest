#include "env.h"
#include <functional>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "implot.h"

namespace env
{
static Env* g_this;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int act, int mods)
{
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(g_this->m, g_this->d);
        mj_forward(g_this->m, g_this->d);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int act, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    glfwGetCursorPos(window, &g_this->lastx_, &g_this->lasty_);
    g_this->button_left_ = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    g_this->button_middle_ = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    g_this->button_right_ = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
}

void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (!g_this->button_left_ && !g_this->button_middle_ && !g_this->button_right_) {
        return;
    }

    double dx = xpos - g_this->lastx_;
    double dy = ypos - g_this->lasty_;
    g_this->lastx_ = xpos;
    g_this->lasty_ = ypos;

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    mjtMouse action;
    if (g_this->button_right_) {
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    } else if (g_this->button_left_) {
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    } else {
        action = mjMOUSE_ZOOM;
    }

    mjv_moveCamera(g_this->m, action, dx / height, dy / height, &g_this->scn_, &g_this->cam_);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    mjv_moveCamera(g_this->m, mjMOUSE_ZOOM, 0, +0.05 * yoffset, &g_this->scn_, &g_this->cam_);
}

Env::Env(std::string const& model_path, int frame_skip, bool show_ui)
    : frame_skip_(frame_skip), show_ui_(show_ui)
{
    g_this = this;
    char error[1000] = {0};
    m = mj_loadXML(model_path.c_str(), nullptr, error, 1000);
    if (!m) {
        THROW_(fmt::format("failed to load xml: {}", error));
    }
    d = mj_makeData(m);
    mj_forward(m, d);
    if (show_ui) {
        std::packaged_task<void()> task([this] { render(); });
        ui_ret_ = std::move(task.get_future());
        std::thread(std::move(task)).detach();
    }
}

Env::~Env()
{
    if (show_ui_) {
        ui_exit_request_ = true;
        ui_ret_.get();
    }
    mj_deleteData(d);
    mj_deleteModel(m);
}

double Env::dt() const { return m->opt.timestep * frame_skip_; }

int Env::actSpace() const { return m->nu; }

int Env::obSpace() const { return m->nq + m->nv; }

torch::Tensor Env::getObserve()
{
    mjtNum* buf = new mjtNum[m->nq + m->nv];
    std::memcpy(buf, d->qpos, sizeof(mjtNum) * m->nq);
    std::memcpy(buf + m->nq, d->qvel, sizeof(mjtNum) * m->nv);
    auto ob = torch::from_blob(
        buf, {1, m->nq + m->nv}, [](void* buf) { delete[] static_cast<mjtNum*>(buf); },
        torch::kFloat64);
    ob = ob.to(torch::kFloat32);
    return ob;
}

void Env::doStep(torch::Tensor action)
{
    assert(action.dim() == 2 && action.size(1) == actSpace());
    std::lock_guard guard(mtx_);
    for (int i = 0; i < action.size(1); ++i) {
        mjtNum min = m->actuator_ctrlrange[2 * i];
        mjtNum max = m->actuator_ctrlrange[2 * i + 1];
        double act = action[0][i].item<double>();
        d->ctrl[i] = (1 - act) / 2.0 * min + (1 + act) / 2.0 * max;
    }
    for (int i = 0; i < frame_skip_; ++i) {
        mj_step(m, d);
    }
}

void Env::report(Progress const& data)
{
    std::lock_guard guard(mtx_);
    progress_data_.push_back(data);
}

void Env::reset(bool clear_progress)
{
    std::lock_guard guard(mtx_);
    if (clear_progress) {
        progress_data_.clear();
    }
    mj_resetData(m, d);
    mj_forward(m, d);
}

void Env::uiSync(std::function<void()> step_func)
{
    double const syncmisalign = 0.1;
    double const refreshfactor = 0.7;
    double cpusync = 0;
    mjtNum simsync = 0;
    while (!uiExited()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        double tmstart = glfwGetTime();
        if (d->time < simsync || tmstart < cpusync || cpusync == 0 ||
            mju_abs((d->time - simsync) - (tmstart - cpusync)) > syncmisalign) {
            cpusync = tmstart;
            simsync = d->time;
            step_func();
        } else {
            while ((d->time - simsync) < (glfwGetTime() - cpusync) &&
                   (glfwGetTime() - tmstart) < refreshfactor / vmode_.refreshRate) {
                mjtNum prevtm = d->time;
                step_func();
                if (d->time < prevtm) {
                    break;
                }
            }
        }
    }
}

bool Env::uiExited() const { return !show_ui_ || (show_ui_ && ui_has_exited_); }

void Env::alignScale()
{
    cam_.lookat[0] = m->stat.center[0];
    cam_.lookat[1] = m->stat.center[1];
    cam_.lookat[2] = m->stat.center[2];
    cam_.distance = 1.5 * m->stat.extent;
    cam_.type = mjCAMERA_FREE;
}

void Env::render()
{
    TRY_;
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    vmode_ = *glfwGetVideoMode(glfwGetPrimaryMonitor());
    window_ = glfwCreateWindow(800, 500, "gym", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        THROW_("failed to create window");
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window_, keyboardCallback);
    glfwSetCursorPosCallback(window_, mouseMoveCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetScrollCallback(window_, scrollCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL2_Init();

    mjv_defaultCamera(&cam_);
    mjv_defaultOption(&opt_);
    mjv_defaultScene(&scn_);
    mjv_makeScene(m, &scn_, 2000);
    mjr_defaultContext(&con_);
    mjr_makeContext(m, &con_, mjFONTSCALE_50);
    alignScale();
    mjv_defaultFigure(&figscore_);
    strncpy(figscore_.yformat, "%.0f", 4);
    figscore_.figurergba[3] = 0.5f;
    figscore_.gridsize[0] = 3;
    figscore_.gridsize[1] = 5;
    mjr_changeFont(50, &con_);

    while (!glfwWindowShouldClose(window_) && !ui_exit_request_) {
        std::unique_lock lock(mtx_);
        glfwPollEvents();
        mjv_updateScene(m, d, &opt_, nullptr, &cam_, mjCAT_ALL, &scn_);
        lock.unlock();
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window_, &viewport.width, &viewport.height);
        mjr_render(viewport, &scn_, &con_);

        if (progress_data_.size() > 0) {
            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(0, viewport.height / 3 * 1.92));
            ImGui::Begin("plot", nullptr,
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration);
            ImPlot::BeginPlot("progress", nullptr, nullptr,
                              ImVec2(viewport.width / 3, viewport.height / 3), ImPlotFlags_NoTitle,
                              ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

            lock.lock();
            auto tries_cb = [](void* data, int idx) {
                return ImPlotPoint{
                    static_cast<double>(idx),
                    static_cast<double>(
                        reinterpret_cast<std::vector<Progress>*>(data)->at(idx).tries)};
            };
            ImPlot::PlotLineG("tries", tries_cb, &progress_data_, progress_data_.size());
            auto score_cb = [](void* data, int idx) {
                return ImPlotPoint{
                    static_cast<double>(idx),
                    reinterpret_cast<std::vector<Progress>*>(data)->at(idx).score_avg};
            };
            ImPlot::PlotLineG("score", score_cb, &progress_data_, progress_data_.size());
            lock.unlock();
            ImPlot::EndPlot();
            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(window_);
    }

    mjv_freeScene(&scn_);
    mjr_freeContext(&con_);

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
    CATCH_;

    ui_has_exited_ = true;
    spdlog::info("render exit");
}

}  // namespace env
