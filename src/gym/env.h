#pragma once
#include "utils/base.h"
#include <torch/torch.h>
#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include <string>
#include <mutex>
#include <future>
#include <thread>

namespace env
{
struct Progress
{
    size_t tries;
    double score_avg;
};

class Env
{
public:
    Env(std::string const& model_path, int frame_skip, bool show_ui);
    virtual ~Env();
    virtual int actSpace() const;
    virtual int obSpace() const;
    virtual torch::Tensor getObserve();
    virtual bool step(torch::Tensor action, double& reward) = 0;
    void report(Progress const& data);
    void reset(bool clear_progress = false);
    void uiSync(std::function<void()> step_func);

protected:
    double dt() const;
    void doStep(torch::Tensor action);

    mjModel* m = nullptr;
    mjData* d = nullptr;

private:
    friend void keyboardCallback(GLFWwindow* window, int key, int scancode, int act, int mods);
    friend void mouseButtonCallback(GLFWwindow* window, int button, int act, int mods);
    friend void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    friend void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void render();
    void alignScale();
    bool uiExited() const;

    mjvCamera cam_;
    mjvOption opt_;
    mjvScene scn_;
    mjrContext con_;
    mjvFigure figscore_;
    GLFWvidmode vmode_;
    bool button_left_ = false;
    bool button_middle_ = false;
    bool button_right_ = false;
    double lastx_ = 0;
    double lasty_ = 0;

    int frame_skip_ = 1;
    bool show_ui_ = true;
    std::mutex mtx_;
    std::future<void> ui_ret_;
    std::atomic<bool> ui_exit_request_ = false;
    std::atomic<bool> ui_has_exited_ = false;
    GLFWwindow* window_ = nullptr;
    std::vector<Progress> progress_data_;
};

}  // namespace env
