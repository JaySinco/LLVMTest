#pragma once
#include "policy.h"

namespace pg
{
struct HyperParams
{
    int hidden;
    int epochs;
    int mini_batch_size;
    double log_std;
    double gamma;
    double lr;
};

class Actor: public torch::nn::Module
{
public:
    Actor(int in, int out, int hidden);
    torch::Tensor forward(torch::Tensor x);

private:
    torch::nn::Linear fc1;
    torch::nn::Linear fc2;
    torch::nn::Linear fc3;
};

class PG: public Policy
{
public:
    PG(int64_t ob_size, int64_t act_size, const HyperParams &hp);
    torch::Tensor get_action(torch::Tensor observe) override;
    void update(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
                torch::Tensor alive) override;

private:
    torch::Tensor calc_returns(torch::Tensor reward, torch::Tensor alive);
    torch::Tensor log_prob(torch::Tensor action, torch::Tensor mu);
    Actor actor;
    torch::optim::Adam opt;
    HyperParams hp;
};

}  // namespace pg
