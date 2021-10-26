#include "mujoco-env.h"
#include <functional>
#include <glog/logging.h>

namespace
{
MujocoEnv *this_;

void keyboard(GLFWwindow *window, int key, int scancode, int act, int mods)
{
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(this_->m, this_->d);
        mj_forward(this_->m, this_->d);
    }
}

void mouse_button(GLFWwindow *window, int button, int act, int mods)
{
    glfwGetCursorPos(window, &this_->lastx, &this_->lasty);
    this_->button_left = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    this_->button_middle = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    this_->button_right = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
}

void mouse_move(GLFWwindow *window, double xpos, double ypos)
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

void scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    mjv_moveCamera(this_->m, mjMOUSE_ZOOM, 0, +0.05 * yoffset, &this_->scn, &this_->cam);
}

}  // namespace

MujocoEnv::MujocoEnv(const std::string &model_path, int frame_skip, bool show_ui)
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
        std::packaged_task<void()> task(std::bind(&MujocoEnv::render, this));
        ui_ret = std::move(task.get_future());
        std::thread(std::move(task)).detach();
    }
}

MujocoEnv::~MujocoEnv()
{
    if (show_ui) {
        ui_exit_request = true;
        ui_ret.get();
    }
    mj_deleteData(d);
    mj_deleteModel(m);
}

void MujocoEnv::step(float *action)
{
    std::lock_guard guard(mtx);
    for (int i = 0; i < m->nu; ++i) {
        mjtNum min = m->actuator_ctrlrange[2 * i];
        mjtNum max = m->actuator_ctrlrange[2 * i + 1];
        d->ctrl[i] = (1 - action[i]) / 2.0 * min + (1 + action[i]) / 2.0 * max;
    }
    for (int i = 0; i < frame_skip; ++i) {
        mj_step(m, d);
    }
}

bool MujocoEnv::ui_exited() { return !show_ui || (show_ui && ui_has_exited); }

void MujocoEnv::render()
{
    if (!glfwInit()) {
        THROW_("failed to init glfw");
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    vmode = *glfwGetVideoMode(glfwGetPrimaryMonitor());
    window = glfwCreateWindow(400, 300, "baselines", nullptr, nullptr);
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
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse_move);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetScrollCallback(window, scroll);

    while (!glfwWindowShouldClose(window) && !ui_exit_request) {
        {
            std::lock_guard guard(mtx);
            glfwPollEvents();
            mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        }
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);
        mjr_render(viewport, &scn, &con);
        glfwSwapBuffers(window);
    }

    mjv_freeScene(&scn);
    mjr_freeContext(&con);
    glfwTerminate();
    ui_has_exited = true;
    LOG(INFO) << "render exit";
}
