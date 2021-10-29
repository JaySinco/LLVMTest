#include "pg.h"
#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace py::literals;

namespace policy
{
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

PG::PG(env::Env &env, const HyperParams &hp)
    : Policy(env),
      actor(env.ob_space(), env.act_space(), hp.hidden),
      opt(actor.parameters(), hp.lr),
      hp(hp)
{
}

void PG::train()
{
    std::atomic<bool> should_abort = false;
    std::thread monitor([&] {
        std::getchar();
        should_abort = true;
    });
    std::vector<double> score_avgs;
    for (int i = 1; i <= hp.max_iters && !should_abort; ++i) {
        double score = 0;
        std::vector<double> scores;
        std::vector<torch::Tensor> observes;
        std::vector<torch::Tensor> actions;
        std::vector<torch::Tensor> rewards;
        std::vector<torch::Tensor> alives;
        for (int s = 0; s < hp.sampling_steps; ++s) {
            auto ob = env.get_observe();
            observes.push_back(ob);
            auto action = get_action(ob);
            actions.push_back(action);
            double reward;
            bool done = env.step(action, reward);
            score += reward;
            rewards.push_back(torch::full({1, 1}, reward));
            alives.push_back(torch::full({1, 1}, done ? 0 : 1));
            if (done) {
                scores.push_back(score);
                score = 0;
                env.reset();
            }
        }
        double score_avg = std::reduce(scores.begin(), scores.end()) / scores.size();
        score_avgs.push_back(score_avg);
        LOG(INFO) << fmt::format("[{:6d}]  score: {:9.3f} / {}", i, score_avg, scores.size());
        learn(torch::cat(observes), torch::cat(actions), torch::cat(rewards), torch::cat(alives));
    }
    py::scoped_interpreter guard{};
    py::module_ plt = py::module_::import("matplotlib.pyplot");
    plt.attr("title")("Average Score");
    plt.attr("plot")(score_avgs, "b");
    plt.attr("show")("block"_a = true);
    monitor.join();
    LOG(INFO) << "training over";
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

void PG::learn(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
               torch::Tensor alive)
{
    auto returns = calc_returns(reward, alive);
    auto dataset = TensorDataset{torch::cat({observe, action}, 1), returns}.map(
        torch::data::transforms::Stack<>());
    auto loader = torch::data::make_data_loader<torch::data::samplers::DistributedRandomSampler>(
        std::move(dataset), hp.minibatch_size);

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
}

}  // namespace pg
}  // namespace policy
