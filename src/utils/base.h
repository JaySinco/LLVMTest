#pragma once
#include <fmt/format.h>
#include <nonstd/expected.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <spdlog/spdlog.h>
#define __DIRNAME__ (std::filesystem::path(__FILE__).parent_path())
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __RESDIR__ (__DIRNAME__ / "res")
#define THROW_(s) throw utils::Error(fmt::format("[{}:{}] {}", __FILENAME__, __LINE__, (s)));
#define TRY_ try {
#define CATCH_                        \
    }                                 \
    catch (const std::exception& err) \
    {                                 \
        spdlog::error(err.what());    \
    }

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
    explicit ScopeExit(T&& t): t{std::move(t)} {}

    ~ScopeExit() { t(); }

    T t;
};

template <typename T>
ScopeExit<T> scopeExit(T&& t)
{
    return ScopeExit<T>{std::move(t)};
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
