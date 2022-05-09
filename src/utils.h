#pragma once
#define BOOST_ALL_NO_LIB
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <nonstd/expected.hpp>
#include <filesystem>
#include <vector>
#include <iostream>
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
    explicit error(const std::string& s): std::runtime_error(s.c_str()){};
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

expected<std::string> readFile(const std::wstring& path);

std::string ws2s(const std::wstring& ws, bool utf8 = false);
std::wstring s2ws(const std::string& s, bool utf8 = false);

std::string base64_encode(const unsigned char* buf, unsigned int bufLen);
std::vector<unsigned char> base64_decode(const std::string& encoded_string);

int word_distance(const std::string& s1, const std::string& s2, bool logStep = false);

}  // namespace utils
