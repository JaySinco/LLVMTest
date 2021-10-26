#pragma once
#include "../../utils.h"
#include "mujoco/mujoco.h"
#include "GLFW/glfw3.h"
#include <string>
#include <mutex>
#include <future>
#include <thread>

class MujocoEnv
{
public:
    MujocoEnv(const std::string &model_path, int frame_skip, bool show_ui);
    ~MujocoEnv();
    void step(float *action);
    bool ui_exited();

    mjModel *m = nullptr;
    mjData *d = nullptr;
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

private:
    void render();

    int frame_skip = 1;
    bool show_ui = true;
    std::mutex mtx;
    std::future<void> ui_ret;
    std::atomic<bool> ui_exit_request = false;
    std::atomic<bool> ui_has_exited = false;
    GLFWwindow *window = nullptr;
};
