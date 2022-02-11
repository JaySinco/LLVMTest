#include "./utils.h"
#include <glog/logging.h>

extern "C" int asm_xor(uint32_t, uint32_t);

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << "1^2=" << asm_xor(1, 2);
}
