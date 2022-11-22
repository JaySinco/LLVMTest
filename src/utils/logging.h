#pragma once
#include "fs.h"
#include "encoding.h"
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#define FSTR(f, ...) (fmt::format(f, __VA_ARGS__))
#define LOG_FSTR(f, ...) (FSTR("[{}:{}] " f, CURR_FILENAME, __LINE__, __VA_ARGS__))
#define MY_THROW(f, ...) throw utils::Error(LOG_FSTR(f, __VA_ARGS__))
#define MY_TRY try {
#define MY_CATCH                      \
    }                                 \
    catch (const std::exception& err) \
    {                                 \
        ELOG(err.what());             \
    }
#define LOG_FUNC(level, ...) SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), level, __VA_ARGS__)
#define VLOG(...) (LOG_FUNC(spdlog::level::debug, __VA_ARGS__))
#define ILOG(...) (LOG_FUNC(spdlog::level::info, __VA_ARGS__))
#define WLOG(...) (LOG_FUNC(spdlog::level::warn, __VA_ARGS__))
#define ELOG(...) (LOG_FUNC(spdlog::level::err, __VA_ARGS__))

namespace utils
{

void initLogger(std::string const& program,
                std::string const& log_dir = ws2s(getExeDir() + L"/logs"));

}  // namespace utils
