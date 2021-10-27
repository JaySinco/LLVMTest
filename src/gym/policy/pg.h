#pragma once
#include "policy.h"
#include "../env/env.h"

class PG: public Policy
{
public:
    PG(const Env &env): net(env.observe_size(), env.action_size()), opt(net.parameters(), 1e-3) {}

    torch::Tensor act_on(torch::Tensor observe, bool is_training) override
    {
        assert(observe.dim() == 2 && observe.size(1) == net.fc1->options.in_features());
        if (is_training) {
            net.train();
            return net.forward(observe);
        } else {
            torch::NoGradGuard no_grad;
            net.eval();
            return net.forward(observe);
        }
    }

    void update(torch::Tensor action, double reward) override
    {
        auto loss = -1 * (net.log_prob(action) * reward).mean();
        opt.zero_grad();
        loss.backward();
        opt.step();
    };

private:
    struct Net: torch::nn::Module
    {
        Net(int64_t n_in, int64_t n_out, double std = 1e-2)
            : fc1(n_in, 50), fc2(50, n_out), log_std(torch::full(n_out, std))
        {
            register_module("fc1", fc1);
            register_module("fc2", fc2);
            register_parameter("log_std", log_std);
        }

        torch::Tensor forward(torch::Tensor x)
        {
            mu = torch::relu(fc1->forward(x));
            mu = torch::tanh(fc2->forward(mu));
            if (this->is_training()) {
                torch::NoGradGuard no_grad;
                return at::normal(mu, log_std.exp().expand_as(mu));
            }
            return mu;
        }

        torch::Tensor log_prob(torch::Tensor action)
        {
            torch::Tensor std_square = (log_std + log_std).exp();
            return -((action - mu) * (action - mu)) / (2 * std_square) - log_std -
                   std::log(std::sqrt(2 * M_PI));
        }

        torch::nn::Linear fc1;
        torch::nn::Linear fc2;
        torch::Tensor mu;
        torch::Tensor log_std;
    };

    Net net;
    torch::optim::Adam opt;
};
