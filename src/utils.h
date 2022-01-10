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
struct error: public std::exception
{
    explicit error(const std::string& s): std::exception(s.c_str()){};
};

template <typename T>
using expected = nonstd::expected<T, error>;

nonstd::unexpected_type<error> make_unexpected(const std::string& s);

std::wstring getExeDir();

expected<std::string> readFile(const std::wstring& path);

std::string ws2s(const std::wstring& ws, bool u8_instead_of_ansi = false);
std::wstring s2ws(const std::string& s, bool u8_instead_of_ansi = false);

std::string base64_encode(const unsigned char* buf, unsigned int bufLen);
std::vector<unsigned char> base64_decode(const std::string& encoded_string);

int word_distance(const std::string& s1, const std::string& s2, bool logStep = false);

}  // namespace utils
