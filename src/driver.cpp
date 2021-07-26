#include "utils.h"
#include "parser.h"

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    ast::line line;
    auto err = parse(L"a,v,\"\"\",sds", line);
    for (const auto &ws: line) {
        LOG(INFO) << utils::ws2s(ws) << "$";
    }
    if (err) {
        LOG(ERROR) << "ERROR => " << err->message;
    }
}
