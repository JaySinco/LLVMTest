#pragma once
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <nonstd/expected.hpp>
#include <filesystem>
#include <vector>
#include <iostream>
#include <string_view>
#define __DIRNAME__ std::filesystem::path(__FILE__).parent_path()
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define THROW_(s) throw utils::error(fmt::format("[{}:{}] {}", __FILENAME__, __LINE__, (s)));
#define TRY_ try {
#define CATCH_ \
    }          \
    catch (const std::exception& err) { LOG(ERROR) << err.what(); }

namespace utils
{
struct error: public std::runtime_error
{
    explicit error(const std::string& s): std::runtime_error(s){};
};

template <typename T>
using expected = nonstd::expected<T, error>;

nonstd::unexpected_type<error> make_unexpected(const std::string& s);

template <typename T>
struct scope_exit
{
    scope_exit(T&& t): t_{std::move(t)} {}
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

std::string ws2s(std::wstring_view ws, bool utf8 = false);
std::wstring s2ws(std::string_view s, bool utf8 = false);

std::string base64_encode(const unsigned char* buf, unsigned int bufLen);
std::vector<unsigned char> base64_decode(std::string_view encoded_string);

int word_distance(std::string_view s1, std::string_view s2, bool logStep = false);

}  // namespace utils
