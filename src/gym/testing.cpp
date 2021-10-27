#include "env/hopper.h"
#include "policy/pg.h"

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    Hopper env(true);
    PG plc(env);
    env.ui_simulate([&]() {
        auto ob = env.get_observe();
        auto action = plc.make_action(ob, true);
        double reward;
        bool done = env.step(action, reward);
        plc.update(action, reward);
        if (done) {
            env.reset();
        }
    });
    CATCH_;
    return 0;
}
