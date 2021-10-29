#include "env/hopper.h"
#include "policy/inaction.h"
#include "policy/pg.h"

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    pg::HyperParams hp;
    hp.max_iters = 15000;
    hp.sampling_steps = 10240;
    hp.minibatch_size = 512;
    hp.epochs = 1;
    hp.hidden = 64;
    hp.log_std = 0;
    hp.gamma = 0.99;
    hp.lr = 3e-4;

    Hopper env;
    pg::PG plc(env, hp);
    plc.train();
    plc.eval();
    CATCH_;
    return 0;
}
