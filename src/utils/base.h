#pragma once
#include <fmt/format.h>
#include <nonstd/expected.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <spdlog/spdlog.h>
#define __DIRNAME__ std::filesystem::path(__FILE__).parent_path()
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define THROW_(s) throw utils::error(fmt::format("[{}:{}] {}", __FILENAME__, __LINE__, (s)));
#define TRY_ try {
#define CATCH_                        \
    }                                 \
    catch (const std::exception& err) \
    {                                 \
        spdlog::error(err.what());    \
    }

namespace utils
{

extern std::filesystem::path const source_repo;

struct error: public std::runtime_error
{
    explicit error(std::string const& s): std::runtime_error(s){};
};

template <typename T>
using expected = nonstd::expected<T, error>;

nonstd::unexpected_type<error> make_unexpected(std::string const& s);

template <typename T>
struct scope_exit
{
    explicit scope_exit(T&& t): t_{std::move(t)} {}
    ~scope_exit() { t_(); }
    T t_;
};

template <typename T>
scope_exit<T> make_scope_exit(T&& t)
{
    return scope_exit<T>{std::move(t)};
}

template <class... Ts>
struct overloaded: Ts...
{
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::wstring getExeDir();

expected<std::string> readFile(std::wstring_view path);

enum class code_page
{
    LOCAL,
    UTF8,
    GBK,
    WCHAR,
};

std::string ws2s(std::wstring_view ws, code_page cp = code_page::LOCAL);
std::wstring s2ws(std::string_view s, code_page cp = code_page::LOCAL);

}  // namespace utils
