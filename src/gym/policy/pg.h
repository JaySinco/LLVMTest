#pragma once
#include "../env/env.h"

class PG
{
public:
    PG(const Env &env);
    torch::Tensor make_action(torch::Tensor observe);
    void update(torch::Tensor observe, torch::Tensor reward);

private:
    struct Net: torch::nn::Module
    {
        Net(int64_t n_in, int64_t n_out, double std = 1e-2);
        torch::Tensor forward(torch::Tensor x);
        torch::Tensor log_prob(torch::Tensor action);

        torch::nn::Linear fc1;
        torch::nn::Linear fc2;
        torch::Tensor mu;
        torch::Tensor log_std;
    };

    Net net;
    torch::optim::Adam opt;
};
