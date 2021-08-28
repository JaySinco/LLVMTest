#include "browser.h"

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    browser br;
    if (!br.open(L"browser")) return -1;
    std::string loaderJs = utils::readFile(L"resources/browser/loader.js").second;

    if (!br.navigate(L"https://www.baidu.com")) return -1;
    br.run_script(utils::s2ws(loaderJs, true));

    std::this_thread::sleep_for(3s);

    if (!br.navigate(L"https://www.bing.com")) return -1;
    br.run_script(utils::s2ws(loaderJs, true));

    br.wait_utill_closed();
    return 0;
}
