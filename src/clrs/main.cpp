#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[])
{
    Catch::Session session;
    auto& config = session.configData();
    config.benchmarkSamples = 10;
    config.benchmarkNoAnalysis = true;
    config.shouldDebugBreak = true;
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) return returnCode;
    int numFailed = session.run();
    return numFailed;
}