#include "pg.h"

namespace pg
{
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

Actor::Actor(int64_t n_in, int64_t n_out, double std)
    : fc1(n_in, 64), fc2(64, n_out), log_std(torch::full(n_out, std))
{
    register_module("fc1", fc1);
    register_module("fc2", fc2);
    register_parameter("log_std", log_std);
}

torch::Tensor Actor::forward(torch::Tensor x)
{
    mu = torch::relu(fc1->forward(x));
    mu = torch::tanh(fc2->forward(mu));
    if (this->is_training()) {
        torch::NoGradGuard no_grad;
        return at::normal(mu, log_std.exp().expand_as(mu));
    }
    return mu;
}

torch::Tensor Actor::log_prob(torch::Tensor action)
{
    torch::Tensor std_square = (log_std + log_std).exp();
    return -((action - mu) * (action - mu)) / (2 * std_square) - log_std -
           std::log(std::sqrt(2 * M_PI));
}

PG::PG(int64_t n_in, int64_t n_out): actor(n_in, n_out), opt(actor.parameters(), 1e-3) {}

torch::Tensor PG::make_action(torch::Tensor observe, bool is_training)
{
    if (is_training) {
        actor.train();
        return actor.forward(observe);
    } else {
        torch::NoGradGuard no_grad;
        actor.eval();
        return actor.forward(observe);
    }
}

torch::Tensor PG::calc_returns(torch::Tensor reward, torch::Tensor done, double gamma)
{
    auto returns = torch::zeros_like(reward);
    double running_returns = 0;
    for (int i = reward.size(0) - 1; i >= 0; --i) {
        running_returns =
            reward[i][0].item<double>() + gamma * running_returns * done[i][0].item<double>();
        returns[i][0] = running_returns;
    }
    returns = (returns - returns.mean()) / returns.std();
    return returns;
}

void PG::update(torch::Tensor observe, torch::Tensor reward, torch::Tensor done)
{
    int epochs = 5;
    int mini_batch_size = 128;
    auto returns = calc_returns(reward, done);
    auto dataset = TensorDataset{observe, returns}.map(torch::data::transforms::Stack<>());
    auto loader = torch::data::make_data_loader<torch::data::samplers::DistributedRandomSampler>(
        std::move(dataset), mini_batch_size);

    for (int e = 0; e < epochs; ++e) {
        for (auto &batch: *loader) {
            actor.train();
            auto action = actor.forward(batch.data);
            auto loss = -1 * (actor.log_prob(action) * batch.target).mean();
            opt.zero_grad();
            loss.backward();
            opt.step();
        }
    }
};

}  // namespace pg
