#include "env/mujoco.h"
#include "prec.h"

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
        x = torch::tanh(fc2->forward(x));
        return x;
    }

    torch::nn::Linear fc1;
    torch::nn::Linear fc2;
};

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    // check command-line arguments
    if (argc > 2) {
        printf(" USAGE:  basic [modelfile]\n");
        return 0;
    }
    std::string arg1 = (__DIRNAME__ / "xml/hopper.xml").string();
    const char *filename = (argc > 1) ? argv[1] : arg1.c_str();
    MuJoCo env(filename, 4, true);

    // init network
    LOG(INFO) << "generalized-coordinates: " << env.m->nq;
    LOG(INFO) << "degrees-of-freedom: " << env.m->nv;
    LOG(INFO) << "controls: " << env.m->nu;
    Net model(env.m->nq + env.m->nv, env.m->nu);
    torch::NoGradGuard no_grad;
    model.eval();

    while (!env.ui_exited()) {
        mjtNum simstart = env.d->time;
        while (env.d->time - simstart < 1.0 / 60.0) {
            mjtNum *buf = new mjtNum[env.m->nq + env.m->nv]{0};
            std::memcpy(buf, env.d->qpos, sizeof(mjtNum) * env.m->nq);
            std::memcpy(buf + env.m->nq, env.d->qvel, sizeof(mjtNum) * env.m->nv);
            auto state = torch::from_blob(
                buf, {1, env.m->nq + env.m->nv}, [](void *buf) { delete[](mjtNum *) buf; },
                torch::kFloat64);
            state = state.to(torch::kFloat32);
            auto act = model.forward(state);
            env.do_simulation((float *)act.data_ptr());
        }
    }
    CATCH_
    return 1;
}
