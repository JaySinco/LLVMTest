#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <codecvt>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "base.h"
#include <fstream>
#include <sstream>

namespace utils
{
std::string ws2s(std::wstring_view ws, code_page cp)
{
#ifdef __linux__
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(ws.data(), ws.data() + ws.size());
#elif _WIN32
    UINT page = cp == UTF8 ? CP_UTF8 : CP_ACP;
    int len = WideCharToMultiByte(page, 0, ws.data(), ws.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string s(len, '\0');
    WideCharToMultiByte(page, 0, ws.data(), ws.size(), s.data(), len, nullptr, nullptr);
    return s;
#endif
}

std::wstring s2ws(std::string_view s, code_page cp)
{
#ifdef __linux__
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s.data(), s.data() + s.size());
#elif _WIN32
    UINT page = cp == UTF8 ? CP_UTF8 : CP_ACP;
    int len = MultiByteToWideChar(page, 0, s.data(), s.size(), nullptr, 0);
    if (len <= 0) return L"";
    std::wstring ws(len, L'\0');
    MultiByteToWideChar(page, 0, s.data(), s.size(), ws.data(), len);
    return ws;
#endif
}

nonstd::unexpected_type<error> make_unexpected(const std::string& s)
{
    return nonstd::unexpected_type<error>(s);
}

expected<std::string> readFile(std::wstring_view path)
{
#ifdef __linux__
    std::ifstream in_file(ws2s(path));
#elif _WIN32
    std::ifstream in_file(path);
#endif
    if (!in_file) {
        return make_unexpected("failed to open file");
    }
    std::stringstream ss;
    ss << in_file.rdbuf();
    return ss.str();
}

std::wstring getExeDir()
{
#ifdef __linux__
    char cep[PATH_MAX] = {0};
    readlink("/proc/self/exe", cep, PATH_MAX);
    return s2ws(dirname(cep));
#elif _WIN32
    wchar_t buf[MAX_PATH + 1] = {0};
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    (wcsrchr(buf, L'\\'))[0] = 0;
    return buf;
#endif
}

}  // namespace utils
