#pragma once
#include "policy.h"

namespace policy
{
namespace pg
{
struct HyperParams
{
    int max_iters;
    int sampling_steps;
    int minibatch_size;
    int epochs;
    int hidden;
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
    PG(env::Env& env, HyperParams const& hp);
    torch::Tensor get_action(torch::Tensor observe) override;
    void train() override;

private:
    torch::Tensor calc_returns(torch::Tensor reward, torch::Tensor alive);
    torch::Tensor log_prob(torch::Tensor action, torch::Tensor mu);
    void learn(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
               torch::Tensor alive);

    Actor actor;
    torch::optim::Adam opt;
    HyperParams hp;
};

}  // namespace pg
}  // namespace policy
