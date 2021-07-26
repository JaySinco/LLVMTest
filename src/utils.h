#pragma once
#define BOOST_ALL_NO_LIB
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <fmt/format.h>
#include <chrono>

using namespace fmt::literals;
using namespace std::chrono_literals;
using namespace std::string_literals;

namespace utils
{
std::string ws2s(const std::wstring &ws, bool u8_instead_of_ansi = false);
std::wstring s2ws(const std::string &s, bool u8_instead_of_ansi = false);
std::pair<bool, std::string> read_file(const std::wstring &path);

}  // namespace utils
