#include "env/hopper.h"

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

    torch::Tensor act(torch::Tensor observe)
    {
        assert(observe.dim() == 2 && observe.size(1) == fc1->options.in_features());
        torch::NoGradGuard no_grad;
        this->eval();
        return this->forward(observe);
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
    HopperEnv env(true);
    Net model(env.observe_size(), env.action_size());

    env.ui_simulate([&]() {
        auto observe = env.get_observe();
        double reward;
        env.step(model.act(observe), reward);
    });
    CATCH_;
    return 0;
}
