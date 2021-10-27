#pragma once
#include "../../utils.h"
#include "../prec.h"
#include "mujoco/mujoco.h"
#include "GLFW/glfw3.h"
#include <string>
#include <mutex>
#include <future>
#include <thread>

class Env
{
public:
    Env(const std::string &model_path, int frame_skip, bool show_ui);
    virtual ~Env();
    virtual int action_size() const;
    virtual int observe_size() const;
    virtual torch::Tensor get_observe();
    virtual bool step(torch::Tensor action, double &reward) = 0;
    void do_step(torch::Tensor action);
    void reset();
    void ui_simulate(std::function<void()> step_func);
    bool ui_exited() const;
    double dt() const;

protected:
    mjModel *m = nullptr;
    mjData *d = nullptr;

private:
    friend void glfw_cb_keyboard(GLFWwindow *window, int key, int scancode, int act, int mods);
    friend void glfw_cb_mouse_button(GLFWwindow *window, int button, int act, int mods);
    friend void glfw_cb_mouse_move(GLFWwindow *window, double xpos, double ypos);
    friend void glfw_cb_scroll(GLFWwindow *window, double xoffset, double yoffset);

    void render();
    void align_scale();

    mjvCamera cam;
    mjvOption opt;
    mjvScene scn;
    mjrContext con;
    GLFWvidmode vmode;
    bool button_left = false;
    bool button_middle = false;
    bool button_right = false;
    double lastx = 0;
    double lasty = 0;

    int frame_skip = 1;
    bool show_ui = true;
    std::mutex mtx;
    std::future<void> ui_ret;
    std::atomic<bool> ui_exit_request = false;
    std::atomic<bool> ui_has_exited = false;
    GLFWwindow *window = nullptr;
};
