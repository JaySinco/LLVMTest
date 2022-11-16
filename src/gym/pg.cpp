#include "pg.h"
#include <torch/torch.h>

namespace policy::pg
{

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

Actor::Actor(int in, int out, int hidden): fc1_(in, hidden), fc2_(hidden, hidden), fc3_(hidden, out)
{
    register_module("fc1", fc1_);
    register_module("fc2", fc2_);
    register_module("fc3", fc3_);
}

torch::Tensor Actor::forward(torch::Tensor x)

{
    auto mu = torch::relu(fc1_->forward(x));
    mu = torch::relu(fc2_->forward(mu));
    mu = torch::tanh(fc3_->forward(mu));
    return mu;
}

struct PG::Impl
{
    Impl(env::Env& env, HyperParams const& hp)
        : actor(env.obSpace(), env.actSpace(), hp.hidden), opt(actor.parameters(), hp.lr), hp(hp)
    {
    }

    Actor actor;
    torch::optim::Adam opt;
    HyperParams hp;
};

PG::PG(env::Env& env, HyperParams const& hp): Policy(env), impl_(new Impl(env, hp)) {}

PG::~PG() { delete impl_; }

void PG::train()
{
    std::atomic<bool> should_abort = false;
    std::thread monitor([&] {
        std::cout << "press enter to end training..." << std::flush;
        std::getchar();
        should_abort = true;
    });
    env.reset(true);
    for (int i = 1; i <= impl_->hp.max_iters && !should_abort; ++i) {
        double score = 0;
        std::vector<double> scores;
        std::vector<torch::Tensor> observes;
        std::vector<torch::Tensor> actions;
        std::vector<torch::Tensor> rewards;
        std::vector<torch::Tensor> alives;
        for (int s = 0; s < impl_->hp.sampling_steps; ++s) {
            auto ob = env.getObserve();
            observes.push_back(ob);
            auto action = getAction(ob);
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
        env.report({scores.size(), std::reduce(scores.begin(), scores.end()) / scores.size()});
        learn(torch::cat(observes), torch::cat(actions), torch::cat(rewards), torch::cat(alives));
    }
    monitor.join();
    ILOG("training over");
}

torch::Tensor PG::getAction(torch::Tensor observe)
{
    torch::NoGradGuard no_grad;
    impl_->actor.eval();
    auto mu = impl_->actor.forward(observe);
    auto log_std = torch::full(mu.sizes(), impl_->hp.log_std);
    return at::normal(mu, log_std.exp());
}

torch::Tensor PG::calcReturns(torch::Tensor reward, torch::Tensor alive) const
{
    auto returns = torch::zeros_like(reward);
    double running_returns = 0;
    for (int i = reward.size(0) - 1; i >= 0; --i) {
        running_returns = reward[i][0].item<double>() +
                          impl_->hp.gamma * running_returns * alive[i][0].item<double>();
        returns[i][0] = running_returns;
    }
    return returns;
}

torch::Tensor PG::logProb(torch::Tensor action, torch::Tensor mu) const
{
    auto log_std = torch::full_like(mu, impl_->hp.log_std);
    auto var = (log_std + log_std).exp();
    auto density = -(action - mu).pow(2) / (2 * var) - log_std - std::log(std::sqrt(2 * M_PI));
    return density.sum(1, true);
}

void PG::learn(torch::Tensor observe, torch::Tensor action, torch::Tensor reward,
               torch::Tensor alive)
{
    auto returns = calcReturns(reward, alive);
    auto dataset = TensorDataset{torch::cat({observe, action}, 1), returns}.map(
        torch::data::transforms::Stack<>());
    auto loader = torch::data::make_data_loader<torch::data::samplers::DistributedRandomSampler>(
        std::move(dataset), impl_->hp.minibatch_size);

    impl_->actor.train();
    for (int e = 0; e < impl_->hp.epochs; ++e) {
        for (auto& batch: *loader) {
            auto ob = batch.data.slice(1, 0, observe.size(1));
            auto act = batch.data.slice(1, observe.size(1));
            auto mu = impl_->actor.forward(ob);
            auto loss = -1 * (logProb(act, mu) * batch.target).mean();
            impl_->opt.zero_grad();
            loss.backward();
            impl_->opt.step();
        }
    }
}

}  // namespace policy::pg
