#pragma once
#include "policy.h"

namespace policy::pg
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
    torch::nn::Linear fc1_;
    torch::nn::Linear fc2_;
    torch::nn::Linear fc3_;
};

class PG: public Policy
{
public:
    PG(env::Env& env, HyperParams const& hp);
    torch::Tensor getAction(torch::Tensor observe) override;
    void train() override;

private:
    torch::Tensor calcReturns(torch::Tensor reward, torch::Tensor alive) const;
    torch::Tensor logProb(torch::Tensor action, torch::Tensor mu) const;
    void learn(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
               torch::Tensor alive);

    Actor actor_;
    torch::optim::Adam opt_;
    HyperParams hp_;
};

}  // namespace policy::pg
