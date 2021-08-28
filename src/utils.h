#pragma once
#define BOOST_ALL_NO_LIB
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <chrono>

using namespace fmt::literals;
using namespace std::chrono_literals;
using namespace std::string_literals;

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define THROW_(s) throw utils::error("[{}:{}] {}"_format(__FILENAME__, __LINE__, (s)));
#define TRY_ try {
#define CATCH_ \
    }          \
    catch (const std::exception &err) { LOG(ERROR) << err.what(); }

namespace utils
{
struct error: public std::exception
{
    error(const std::string &s): std::exception(s.c_str()){};
};

std::string ws2s(const std::wstring &ws, bool u8_instead_of_ansi = false);
std::wstring s2ws(const std::string &s, bool u8_instead_of_ansi = false);
std::pair<bool, std::string> readFile(const std::wstring &path);
std::wstring getExePath();

}  // namespace utils
