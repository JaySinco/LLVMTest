#include "../utils.h"
#include "prec.h"
#include "mujoco/mujoco.h"
#include "GLFW/glfw3.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// MuJoCo data structures
mjModel *m = NULL;  // MuJoCo model
mjData *d = NULL;   // MuJoCo data
mjvCamera cam;      // abstract camera
mjvOption opt;      // visualization options
mjvScene scn;       // abstract scene
mjrContext con;     // custom GPU context

// mouse interaction
bool button_left = false;
bool button_middle = false;
bool button_right = false;
double lastx = 0;
double lasty = 0;

// keyboard callback
void keyboard(GLFWwindow *window, int key, int scancode, int act, int mods)
{
    // backspace: reset simulation
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(m, d);
        mj_forward(m, d);
    }
}

// mouse button callback
void mouse_button(GLFWwindow *window, int button, int act, int mods)
{
    // update button state
    button_left = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    button_middle = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    button_right = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    // update mouse position
    glfwGetCursorPos(window, &lastx, &lasty);
}

// mouse move callback
void mouse_move(GLFWwindow *window, double xpos, double ypos)
{
    // no buttons down: nothing to do
    if (!button_left && !button_middle && !button_right) return;

    // compute mouse displacement, save
    double dx = xpos - lastx;
    double dy = ypos - lasty;
    lastx = xpos;
    lasty = ypos;

    // get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // get shift key state
    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // determine action based on mouse button
    mjtMouse action;
    if (button_right)
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    else if (button_left)
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    else
        action = mjMOUSE_ZOOM;

    // move camera
    mjv_moveCamera(m, action, dx / height, dy / height, &scn, &cam);
}

// scroll callback
void scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    // emulate vertical mouse motion = 5% of window height
    mjv_moveCamera(m, mjMOUSE_ZOOM, 0, +0.05 * yoffset, &scn, &cam);
}

struct Net: torch::nn::Module
{
    Net(int64_t in_features, int64_t out_features): fc1(in_features, 50), fc2(50, out_features)
    {
        register_module("fc1", fc1);
        register_module("fc2", fc2);
    }

    torch::Tensor forward(torch::Tensor x)
    {
        x = torch::relu(fc1->forward(x));
        x = torch::sigmoid(fc2->forward(x));
        return x;
    }

    torch::nn::Linear fc1;
    torch::nn::Linear fc2;
};

// main function
int main(int argc, const char **argv)
{
    TRY_;
    // check command-line arguments
    if (argc > 2) {
        printf(" USAGE:  basic [modelfile]\n");
        return 0;
    }
    std::string arg1 = (__DIRNAME__ / "MJCF/hopper.xml").string();
    const char *filename = (argc > 1) ? argv[1] : arg1.c_str();

    // load and compile model
    char error[1000] = "Could not load binary model";
    if (strlen(filename) > 4 && !strcmp(filename + strlen(filename) - 4, ".mjb"))
        m = mj_loadModel(filename, 0);
    else
        m = mj_loadXML(filename, 0, error, 1000);
    if (!m) mju_error_s("Load model error: %s", error);

    // make data
    d = mj_makeData(m);

    // init network
    LOG(INFO) << "generalized-coordinates: " << m->nq;
    LOG(INFO) << "degrees-of-freedom: " << m->nv;
    LOG(INFO) << "controls: " << m->nu;
    Net model(m->nq + m->nv, m->nu);
    torch::NoGradGuard no_grad;
    model.eval();

    // init GLFW
    if (!glfwInit()) mju_error("Could not initialize GLFW");

    // create window, make OpenGL context current, request v-sync
    GLFWwindow *window = glfwCreateWindow(1200, 900, "Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // initialize visualization data structures
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);

    // create scene and context
    mjv_makeScene(m, &scn, 2000);
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    // install GLFW mouse and keyboard callbacks
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse_move);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetScrollCallback(window, scroll);

    // run main loop, target real-time simulation and 60 fps rendering
    while (!glfwWindowShouldClose(window)) {
        // advance interactive simulation for 1/60 sec
        //  Assuming MuJoCo can simulate faster than real-time, which it usually can,
        //  this loop will finish on time for the next frame to be rendered at 60 fps.
        //  Otherwise add a cpu timer and exit this loop when it is time to render.
        mjtNum simstart = d->time;
        while (d->time - simstart < 1.0 / 60.0) {
            mj_step(m, d);
            mjtNum *buf = new mjtNum[m->nq + m->nv]{0};
            std::memcpy(buf, d->qpos, sizeof(mjtNum) * m->nq);
            std::memcpy(buf + m->nq, d->qvel, sizeof(mjtNum) * m->nv);
            auto state = torch::from_blob(
                buf, {1, m->nq + m->nv}, [](void *buf) { delete[](mjtNum *) buf; },
                torch::kFloat64);
            state = state.to(torch::kFloat32);
            auto act = model.forward(state);
            for (int i = 0; i < m->nu; ++i) {
                mjtNum min = m->actuator_ctrlrange[2 * i];
                mjtNum max = m->actuator_ctrlrange[2 * i + 1];
                double factor = act[0][i].item<double>();
                d->ctrl[i] = min + (max - min) * factor;
            }
        }

        // get framebuffer viewport
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

        // update scene and render
        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        // swap OpenGL buffers (blocking call due to v-sync)
        glfwSwapBuffers(window);

        // process pending GUI events, call GLFW callbacks
        glfwPollEvents();
    }

    // free visualization storage
    mjv_freeScene(&scn);
    mjr_freeContext(&con);

    // free MuJoCo model and data
    mj_deleteData(d);
    mj_deleteModel(m);

// terminate GLFW (crashes with Linux NVidia drivers)
#if defined(__APPLE__) || defined(_WIN32)
    glfwTerminate();
#endif
    CATCH_
    return 1;
}
