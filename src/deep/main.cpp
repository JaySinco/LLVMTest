#include "prec.h"
#include "../utils.h"

DEFINE_bool(lr, false, "linear regression");
DEFINE_bool(fm, false, "fashion mnist");

void linear_regression();
void fashion_mnist();

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::SetUsageMessage("[[ flags ]]");

    TRY_;
    torch::manual_seed(1);
    if (FLAGS_lr) {
        linear_regression();
    } else if (FLAGS_fm) {
        fashion_mnist();
    } else {
        google::ShowUsageWithFlagsRestrict(argv[0], "deep");
    }
    CATCH_;
    return 0;
}
