#include "env/hopper.h"
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

    std::vector<double> act(std::vector<double> &observe)
    {
        assert(observe.size() == fc1->options.in_features());
        torch::NoGradGuard no_grad;
        this->eval();
        auto state = torch::from_blob(
            observe.data(), {1, (int64_t)observe.size()}, [](void *) {}, torch::kFloat64);
        state = state.to(torch::kFloat32);
        auto out = this->forward(state);
        float *beg = (float *)out.data_ptr();
        float *end = beg + fc2->options.out_features();
        return {beg, end};
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
    LOG(INFO) << fmt::format("observe={}, action={}", env.observe_dim(), env.action_dim());
    Net model(env.observe_dim(), env.action_dim());

    env.ui_simulate([&]() {
        auto observe = env.get_observe();
        double reward;
        env.step(model.act(observe), reward);
    });
    CATCH_;
    return 0;
}
