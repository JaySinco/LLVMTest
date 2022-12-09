#include "fs.h"
#include "encoding.h"
#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <fstream>
#include <sstream>

namespace utils
{

std::filesystem::path sourceRepo() { return std::filesystem::path(_SOURCE_REPO); }

std::wstring getExeDir()
{
#ifdef __linux__
    char cep[PATH_MAX] = {0};
    readlink("/proc/self/exe", cep, PATH_MAX);
    return s2ws(dirname(cep));
#elif _WIN32
    wchar_t buf[MAX_PATH + 1] = {0};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    (wcsrchr(buf, L'\\'))[0] = 0;
    return buf;
#endif
}

std::wstring defaultLoggingDir()
{
    auto path = std::filesystem::path(getExeDir()) / "logs";
    return path.generic_wstring();
}

Expected<std::string> readFile(std::wstring_view path)
{
#ifdef __linux__
    std::ifstream in_file(ws2s(path));
#elif _WIN32
    std::ifstream in_file(path);
#endif
    if (!in_file) {
        return unexpected("failed to open file");
    }
    std::stringstream ss;
    ss << in_file.rdbuf();
    return ss.str();
}

}  // namespace utils
