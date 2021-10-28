#include "pg.h"

namespace pg
{
Actor::Actor(int in, int out, int hidden): fc1(in, hidden), fc2(hidden, hidden), fc3(hidden, out)
{
    register_module("fc1", fc1);
    register_module("fc2", fc2);
    register_module("fc3", fc3);
}

torch::Tensor Actor::forward(torch::Tensor x)
{
    auto mu = torch::relu(fc1->forward(x));
    mu = torch::relu(fc2->forward(mu));
    mu = torch::tanh(fc3->forward(mu));
    return mu;
}

PG::PG(int64_t ob_size, int64_t act_size, const HyperParams &hp)
    : actor(ob_size, act_size, hp.hidden), opt(actor.parameters(), hp.lr), hp(hp)
{
}

torch::Tensor PG::get_action(torch::Tensor observe)
{
    torch::NoGradGuard no_grad;
    actor.eval();
    auto mu = actor.forward(observe);
    auto log_std = torch::full(mu.sizes(), hp.log_std);
    return at::normal(mu, log_std.exp());
}

torch::Tensor PG::calc_returns(torch::Tensor reward, torch::Tensor alive)
{
    auto returns = torch::zeros_like(reward);
    double running_returns = 0;
    for (int i = reward.size(0) - 1; i >= 0; --i) {
        running_returns =
            reward[i][0].item<double>() + hp.gamma * running_returns * alive[i][0].item<double>();
        returns[i][0] = running_returns;
    }
    return returns;
}

torch::Tensor PG::log_prob(torch::Tensor action, torch::Tensor mu)
{
    auto log_std = torch::full_like(mu, hp.log_std);
    auto var = (log_std + log_std).exp();
    auto density = -(action - mu).pow(2) / (2 * var) - log_std - std::log(std::sqrt(2 * M_PI));
    return density.sum(1, true);
}

void PG::update(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
                torch::Tensor alive)
{
    auto returns = calc_returns(reward, alive);
    auto dataset = TensorDataset{torch::cat({observe, action}, 1), returns}.map(
        torch::data::transforms::Stack<>());
    auto loader = torch::data::make_data_loader<torch::data::samplers::DistributedRandomSampler>(
        std::move(dataset), hp.mini_batch_size);

    actor.train();
    for (int e = 0; e < hp.epochs; ++e) {
        for (auto &batch: *loader) {
            auto ob = batch.data.slice(1, 0, observe.size(1));
            auto act = batch.data.slice(1, observe.size(1));
            auto mu = actor.forward(ob);
            auto loss = -1 * (log_prob(act, mu) * batch.target).mean();
            opt.zero_grad();
            loss.backward();
            opt.step();
        }
    }
};

}  // namespace pg
