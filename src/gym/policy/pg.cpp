#include "pg.h"

struct TensorDataset: public torch::data::Dataset<TensorDataset>
{
    TensorDataset(torch::Tensor data, torch::Tensor target)
        : data(std::move(data)), target(std::move(target))
    {
    }

    ExampleType get(size_t index) override { return {data[index], target[index]}; }

    c10::optional<size_t> size() const override { return data.size(0); }

    torch::Tensor data;
    torch::Tensor target;
};

PG::PG(const Env &env): net(env.observe_size(), env.action_size()), opt(net.parameters(), 1e-3) {}

torch::Tensor PG::make_action(torch::Tensor observe)
{
    assert(observe.dim() == 2 && observe.size(1) == net.fc1->options.in_features());
    torch::NoGradGuard no_grad;
    net.eval();
    auto action = net.forward(observe);
    return action;
}

void PG::update(torch::Tensor observe, torch::Tensor reward)
{
    assert(observe.dim() == 2 && observe.size(1) == net.fc1->options.in_features());
    assert(reward.dim() == 2 && reward.size(1) == 1);
    net.train();
    auto action = net.forward(observe);
    auto loss = -1 * (net.log_prob(action) * reward).mean();
    opt.zero_grad();
    loss.backward();
    opt.step();
};

PG::Net::Net(int64_t n_in, int64_t n_out, double std)
    : fc1(n_in, 50), fc2(50, n_out), log_std(torch::full(n_out, std))
{
    register_module("fc1", fc1);
    register_module("fc2", fc2);
    register_parameter("log_std", log_std);
}

torch::Tensor PG::Net::forward(torch::Tensor x)
{
    mu = torch::relu(fc1->forward(x));
    mu = torch::tanh(fc2->forward(mu));
    if (this->is_training()) {
        torch::NoGradGuard no_grad;
        return at::normal(mu, log_std.exp().expand_as(mu));
    }
    return mu;
}

torch::Tensor PG::Net::log_prob(torch::Tensor action)
{
    torch::Tensor std_square = (log_std + log_std).exp();
    return -((action - mu) * (action - mu)) / (2 * std_square) - log_std -
           std::log(std::sqrt(2 * M_PI));
}
