#include "env.h"
#include <functional>

namespace env
{
static Env *this_;

static void glfw_cb_keyboard(GLFWwindow *window, int key, int scancode, int act, int mods)
{
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(this_->m, this_->d);
        mj_forward(this_->m, this_->d);
    }
}

static void glfw_cb_mouse_button(GLFWwindow *window, int button, int act, int mods)
{
    glfwGetCursorPos(window, &this_->lastx, &this_->lasty);
    this_->button_left = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    this_->button_middle = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    this_->button_right = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
}

static void glfw_cb_mouse_move(GLFWwindow *window, double xpos, double ypos)
{
    if (!this_->button_left && !this_->button_middle && !this_->button_right) {
        return;
    }

    double dx = xpos - this_->lastx;
    double dy = ypos - this_->lasty;
    this_->lastx = xpos;
    this_->lasty = ypos;

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    mjtMouse action;
    if (this_->button_right)
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    else if (this_->button_left)
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    else
        action = mjMOUSE_ZOOM;

    mjv_moveCamera(this_->m, action, dx / height, dy / height, &this_->scn, &this_->cam);
}

static void glfw_cb_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    mjv_moveCamera(this_->m, mjMOUSE_ZOOM, 0, +0.05 * yoffset, &this_->scn, &this_->cam);
}

Env::Env(const std::string &model_path, int frame_skip, bool show_ui)
    : frame_skip(frame_skip), show_ui(show_ui)
{
    this_ = this;
    char error[1000] = {0};
    m = mj_loadXML(model_path.c_str(), nullptr, error, 1000);
    if (!m) {
        THROW_(fmt::format("failed to load xml: {}", error));
    }
    d = mj_makeData(m);
    mj_forward(m, d);
    if (show_ui) {
        std::packaged_task<void()> task(std::bind(&Env::render, this));
        ui_ret = std::move(task.get_future());
        std::thread(std::move(task)).detach();
    }
}

Env::~Env()
{
    if (show_ui) {
        ui_exit_request = true;
        ui_ret.get();
    }
    mj_deleteData(d);
    mj_deleteModel(m);
}

double Env::dt() const { return m->opt.timestep * frame_skip; }

int Env::act_space() const { return m->nu; }

int Env::ob_space() const { return m->nq + m->nv; }

torch::Tensor Env::get_observe()
{
    mjtNum *buf = new mjtNum[m->nq + m->nv];
    std::memcpy(buf, d->qpos, sizeof(mjtNum) * m->nq);
    std::memcpy(buf + m->nq, d->qvel, sizeof(mjtNum) * m->nv);
    auto ob = torch::from_blob(
        buf, {1, m->nq + m->nv}, [](void *buf) { delete[](mjtNum *) buf; }, torch::kFloat64);
    ob = ob.to(torch::kFloat32);
    return ob;
}

void Env::do_step(torch::Tensor action)
{
    assert(action.dim() == 2 && action.size(1) == act_space());
    std::lock_guard guard(mtx);
    for (int i = 0; i < action.size(1); ++i) {
        mjtNum min = m->actuator_ctrlrange[2 * i];
        mjtNum max = m->actuator_ctrlrange[2 * i + 1];
        double act = action[0][i].item<double>();
        d->ctrl[i] = (1 - act) / 2.0 * min + (1 + act) / 2.0 * max;
    }
    for (int i = 0; i < frame_skip; ++i) {
        mj_step(m, d);
    }
}

void Env::reset()
{
    std::lock_guard guard(mtx);
    mj_resetData(m, d);
    mj_forward(m, d);
}

void Env::ui_sync(std::function<void()> step_func)
{
    const double syncmisalign = 0.1;
    const double refreshfactor = 0.7;
    double cpusync = 0;
    mjtNum simsync = 0;
    while (!ui_exited()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        double tmstart = glfwGetTime();
        if (d->time < simsync || tmstart < cpusync || cpusync == 0 ||
            mju_abs((d->time - simsync) - (tmstart - cpusync)) > syncmisalign) {
            cpusync = tmstart;
            simsync = d->time;
            step_func();
        } else {
            while ((d->time - simsync) < (glfwGetTime() - cpusync) &&
                   (glfwGetTime() - tmstart) < refreshfactor / vmode.refreshRate) {
                mjtNum prevtm = d->time;
                step_func();
                if (d->time < prevtm) break;
            }
        }
    }
}

void Env::insert_scores(std::initializer_list<double> data)
{
    std::lock_guard guard(mtx);
    scores.insert(scores.end(), data.begin(), data.end());
}

void Env::clear_scores()
{
    std::lock_guard guard(mtx);
    scores.clear();
}

bool Env::ui_exited() const { return !show_ui || (show_ui && ui_has_exited); }

void Env::align_scale()
{
    cam.lookat[0] = m->stat.center[0];
    cam.lookat[1] = m->stat.center[1];
    cam.lookat[2] = m->stat.center[2];
    cam.distance = 1.5 * m->stat.extent;
    cam.type = mjCAMERA_FREE;
}

void Env::render()
{
    TRY_;
    if (!glfwInit()) {
        THROW_("failed to init glfw");
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    vmode = *glfwGetVideoMode(glfwGetPrimaryMonitor());
    window = glfwCreateWindow(480, 300, "gym", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        THROW_("failed to create window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjv_makeScene(m, &scn, 2000);
    mjr_defaultContext(&con);
    mjr_makeContext(m, &con, mjFONTSCALE_50);
    align_scale();
    mjv_defaultFigure(&figscore);
    strcpy(figscore.yformat, "%.0f");
    figscore.figurergba[3] = 0.5f;
    figscore.gridsize[0] = 3;
    figscore.gridsize[1] = 5;
    mjr_changeFont(50, &con);

    glfwSetKeyCallback(window, glfw_cb_keyboard);
    glfwSetCursorPosCallback(window, glfw_cb_mouse_move);
    glfwSetMouseButtonCallback(window, glfw_cb_mouse_button);
    glfwSetScrollCallback(window, glfw_cb_scroll);

    while (!glfwWindowShouldClose(window) && !ui_exit_request) {
        {
            std::lock_guard guard(mtx);
            glfwPollEvents();
            mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
            if (scores.size() > 0) {
                figscore.linepnt[0] = mjMIN(mjMAXLINEPNT, scores.size());
                int beg = mjMAX(0, (int64_t)scores.size() - mjMAXLINEPNT);
                double sc_min = std::numeric_limits<double>::max();
                for (int i = 0; i < figscore.linepnt[0]; ++i) {
                    double sc = scores.at(i + beg);
                    sc_min = std::min(sc_min, sc);
                    figscore.linedata[0][2 * i] = i + beg + 1;
                    figscore.linedata[0][2 * i + 1] = sc;
                }
                figscore.range[0][0] = beg;
                figscore.range[0][1] = 0;
                figscore.range[1][0] = sc_min;
                figscore.range[1][1] = 0;
            }
        }
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);
        mjr_render(viewport, &scn, &con);
        if (scores.size() > 0) {
            mjrRect figview = {0, 0, viewport.width / 3, viewport.height / 3};
            mjr_figure(figview, &figscore, &con);
        }
        glfwSwapBuffers(window);
    }

    mjv_freeScene(&scn);
    mjr_freeContext(&con);
    glfwTerminate();
    CATCH_;

    ui_has_exited = true;
    LOG(INFO) << "render exit";
}

}  // namespace env
