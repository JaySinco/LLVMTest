#include "env/hopper.h"
#include "policy/sloppy.h"
#include "policy/pg.h"

void train(Env &env, Policy &plc, int steps = 2048, int iters = 15000)
{
    for (int i = 1; i <= iters; ++i) {
        double score = 0;
        std::vector<double> scores;
        std::vector<torch::Tensor> observes;
        std::vector<torch::Tensor> rewards;
        std::vector<torch::Tensor> alives;
        for (int s = 0; s < steps; ++s) {
            auto ob = env.get_observe();
            observes.push_back(ob);
            auto action = plc.make_action(ob, true);
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
        plc.update(torch::cat(observes), torch::cat(rewards), torch::cat(alives));
    }
}

void test(Env &env, Policy &plc)
{
    env.ui_sync([&]() {
        auto ob = env.get_observe();
        auto action = plc.make_action(ob, false);
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
    Hopper env(false);
    pg::HyperParams hp;
    pg::PG plc(env.observe_size(), env.action_size(), hp);
    train(env, plc);
    CATCH_;
    return 0;
}
