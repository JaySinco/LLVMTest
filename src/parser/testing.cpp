#include "../utils.h"
#include "parser.h"
#include <glog/logging.h>

int main(int argc, char** argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    parser::parse(__DIRNAME__ / "input.txt");
}
