#include "logging.h"
#include <fmt/chrono.h>
#include <spdlog/details/os.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace utils
{

void initLogger(std::string const& program, std::string const& log_dir)
{
    bool alsologtostderr = true;
    auto minloglevel = spdlog::level::debug;
    auto stderrlevel = spdlog::level::info;
    auto logbuflevel = spdlog::level::err;
    int logbufsecs = 30;
    int maxlogsize = 100;  // MB

    if (spdlog::get(program)) {
        return;
    }
    std::vector<spdlog::sink_ptr> sinks;
    if (!log_dir.empty()) {
        std::filesystem::path fpath(log_dir);
        std::string fname =
            FSTR("{}_{:%Y%m%d.%H%M%S}_{}.log", std::filesystem::path(program).stem().string(),
                 spdlog::details::os::localtime(), spdlog::details::os::pid());
        fpath /= fname;

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            fpath.string(), maxlogsize * 1024 * 1024, 10, true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("%L%m%d %H:%M:%S.%f %t %s:%#] %v");
        sinks.push_back(file_sink);
    }
    if (alsologtostderr) {
        auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        console_sink->set_level(stderrlevel);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
        sinks.push_back(console_sink);
    }

    auto logger = std::make_shared<spdlog::logger>(program, sinks.begin(), sinks.end());
    logger->set_level(minloglevel);
    logger->flush_on(logbuflevel);
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(logbufsecs));

    VLOG("### GIT HASH: {} ###", _GIT_HASH);
    VLOG("### GIT BRANCH: {} ###", _GIT_BRANCH);
    VLOG("### BUILD AT: {} {} ###", __DATE__, __TIME__);
}

}  // namespace utils
