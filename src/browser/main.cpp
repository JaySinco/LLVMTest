#include "browser.h"

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    browser br;
    if (!br.open()) return -1;
    if (!br.navigate(L"https://www.baidu.com/")) return -1;
    br.wait_utill_closed();
    return 0;
}
