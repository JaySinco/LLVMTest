#pragma once
#include "policy.h"

namespace pg
{
class Actor: public torch::nn::Module
{
public:
    Actor(int64_t n_in, int64_t n_out, double std = 1e-2);
    torch::Tensor forward(torch::Tensor x);
    torch::Tensor log_prob(torch::Tensor action);

private:
    torch::nn::Linear fc1;
    torch::nn::Linear fc2;
    torch::Tensor mu;
    torch::Tensor log_std;
};

class PG: public Policy
{
public:
    PG(int64_t n_in, int64_t n_out);
    torch::Tensor make_action(torch::Tensor observe, bool is_training) override;
    void update(torch::Tensor observe, torch::Tensor reward, torch::Tensor alive) override;

private:
    torch::Tensor calc_returns(torch::Tensor reward, torch::Tensor alive, double gamma = 0.99);
    Actor actor;
    torch::optim::Adam opt;
};

}  // namespace pg
