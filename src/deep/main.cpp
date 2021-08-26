#include "prec.h"

void mnist();
void linear_regression();

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    torch::manual_seed(1);
    try {
        linear_regression();
    } catch (std::exception &err) {
        LOG(ERROR) << err.what();
    }
    return 0;
}
