#include "prec.h"
#include "../utils.h"

void mnist();
void linear_regression();

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    torch::manual_seed(1);
    linear_regression();
    CATCH_;
    return 0;
}
