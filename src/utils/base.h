#pragma once
#include <fmt/format.h>
#include <nonstd/expected.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <spdlog/spdlog.h>
#define CURR_DIRNAME (std::filesystem::path(__FILE__).parent_path())
#define CURR_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define CURR_RESDIR (CURR_DIRNAME / "res")
#define FSTR(f, ...) (fmt::format(f, __VA_ARGS__))
#define LOG_FSTR(f, ...) (FSTR("[{}:{}] " f, CURR_FILENAME, __LINE__, __VA_ARGS__))
#define MY_THROW(f, ...) throw utils::Error(LOG_FSTR(f, __VA_ARGS__))
#define MY_TRY try {
#define MY_CATCH                      \
    }                                 \
    catch (const std::exception& err) \
    {                                 \
        ELOG("{}", err.what());       \
    }
#define VLOG(f, ...) (spdlog::debug(LOG_FSTR(f, __VA_ARGS__)))
#define ILOG(f, ...) (spdlog::info(LOG_FSTR(f, __VA_ARGS__)))
#define WLOG(f, ...) (spdlog::warn(LOG_FSTR(f, __VA_ARGS__)))
#define ELOG(f, ...) (spdlog::error(LOG_FSTR(f, __VA_ARGS__)))

namespace utils
{

extern std::filesystem::path const kSourceRepo;

struct Error: public std::runtime_error
{
    explicit Error(std::string const& s): std::runtime_error(s){};
};

template <typename T>
using Expected = nonstd::expected<T, Error>;

nonstd::unexpected_type<Error> unexpected(std::string const& s);

template <typename T>
struct ScopeExit
{
    explicit ScopeExit(T&& t): t(std::move(t)) {}

    ~ScopeExit() { t(); }

    T t;
};

template <typename T>
ScopeExit<T> scopeExit(T&& t)
{
    return ScopeExit<T>(std::move(t));
}

template <class... Ts>
struct Overloaded: Ts...
{
    using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

std::wstring getExeDir();

Expected<std::string> readFile(std::wstring_view path);

enum class CodePage
{
    kLOCAL,
    kUTF8,
    kGBK,
    kWCHAR,
};

std::string ws2s(std::wstring_view ws, CodePage cp = CodePage::kLOCAL);
std::wstring s2ws(std::string_view s, CodePage cp = CodePage::kLOCAL);

}  // namespace utils
