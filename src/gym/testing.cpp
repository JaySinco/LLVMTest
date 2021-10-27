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
        auto action = plc.act_on(ob, true);
        double reward;
        env.step(action, reward);
        plc.update(action, reward);
    });
    CATCH_;
    return 0;
}
