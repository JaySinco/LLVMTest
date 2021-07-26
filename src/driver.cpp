#include "utils.h"
#include "parser.h"

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    ast::expr_value ast;
    auto err = parser::parse("ip > 10", ast);
    if (err) {
        LOG(ERROR) << err->message;
    }
}
