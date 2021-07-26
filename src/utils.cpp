#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "utils.h"
#include <fstream>

namespace utils
{
std::string ws2s(const std::wstring &ws, bool u8_instead_of_ansi)
{
    UINT page = u8_instead_of_ansi ? CP_UTF8 : CP_ACP;
    int len = WideCharToMultiByte(page, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto buf = new char[len]{0};
    WideCharToMultiByte(page, 0, ws.c_str(), -1, buf, len, nullptr, nullptr);
    std::string s = buf;
    delete[] buf;
    return s;
}

std::wstring s2ws(const std::string &s, bool u8_instead_of_ansi)
{
    UINT page = u8_instead_of_ansi ? CP_UTF8 : CP_ACP;
    int len = MultiByteToWideChar(page, 0, s.c_str(), -1, nullptr, 0);
    auto buf = new wchar_t[len]{0};
    MultiByteToWideChar(page, 0, s.c_str(), -1, buf, len);
    std::wstring ws = buf;
    delete[] buf;
    return ws;
}

std::pair<bool, std::string> read_file(const std::wstring &path)
{
    std::ifstream in_file(path);
    if (!in_file) {
        LOG(ERROR) << "failed to read file: {}"_format(ws2s(path));
        return {false, ""};
    }
    std::stringstream ss;
    ss << in_file.rdbuf();
    return {true, ss.str()};
}

}  // namespace utils
