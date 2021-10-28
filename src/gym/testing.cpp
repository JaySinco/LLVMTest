#include "env/hopper.h"
#include "policy/inaction.h"
#include "policy/pg.h"

void train(Env &env, Policy &plc, int sampling_steps, int max_iters)
{
    std::atomic<bool> should_abort = false;
    std::thread monitor([&] {
        std::getchar();
        should_abort = true;
    });
    for (int i = 1; i <= max_iters && !should_abort; ++i) {
        double score = 0;
        std::vector<double> scores;
        std::vector<torch::Tensor> observes;
        std::vector<torch::Tensor> actions;
        std::vector<torch::Tensor> rewards;
        std::vector<torch::Tensor> alives;
        for (int s = 0; s < sampling_steps; ++s) {
            auto ob = env.get_observe();
            observes.push_back(ob);
            auto action = plc.get_action(ob);
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
        LOG(INFO) << fmt::format("iter {}, turns={}, score-avg={:.3f}", i, scores.size(),
                                 score_avg);
        plc.update(torch::cat(observes), torch::cat(actions), torch::cat(rewards),
                   torch::cat(alives));
    }
    LOG(INFO) << "press key to continue...";
    monitor.join();
    LOG(INFO) << "training over";
}

void eval(Env &env, Policy &plc)
{
    env.ui_sync([&]() {
        auto ob = env.get_observe();
        auto action = plc.get_action(ob);
        double reward;
        bool done = env.step(action, reward);
        if (done) {
            env.reset();
        }
    });
}

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    Hopper env(true);
    pg::HyperParams hp;
    hp.hidden = 64;
    hp.epochs = 1;
    hp.mini_batch_size = 512;
    hp.log_std = 0;
    hp.gamma = 0.99;
    hp.lr = 3e-4;
    pg::PG plc(env.observe_size(), env.action_size(), hp);
    train(env, plc, 10240, 15000);
    eval(env, plc);
    CATCH_;
    return 0;
}
