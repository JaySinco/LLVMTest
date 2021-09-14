#include "prec.h"
#include "../utils.h"

void linear_regression();
void fashion_mnist();

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TRY_;
    torch::manual_seed(1);
    // linear_regression();
    fashion_mnist();
    CATCH_;
    return 0;
}
