#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "base.h"
#include <fstream>
#include <sstream>
#include <iconv.h>
#include <libcharset.h>

namespace utils
{

class iconv_wrapper
{
public:
    iconv_wrapper(code_page tocode, code_page fromcode)
    {
        cd = iconv_open(code2str(tocode), code2str(fromcode));
    }

    ~iconv_wrapper()
    {
        if ((iconv_t)-1 == cd) return;
        iconv_close(cd);
    }

    size_t convert(char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
    {
        if ((iconv_t)-1 == cd) return -1;
        return iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
    }

    template <typename T = std::string>
    T convert(std::string_view in, size_t max_outbytes)
    {
        char* inbuf = (char*)in.data();
        size_t inbytes = in.size();
        size_t outbytes = max_outbytes;
        std::unique_ptr<char> buf(new char[outbytes]{0});
        char* outbuf = buf.get();
        size_t ret = convert(&inbuf, &inbytes, &outbuf, &outbytes);
        if ((size_t)-1 == ret) {
            return {};
        }
        using CharT = typename T::value_type;
        return T((CharT*)buf.get(), (CharT*)outbuf);
    }

    static char const* code2str(code_page cp)
    {
        switch (cp) {
            case code_page::LOCAL:
                setlocale(LC_ALL, "");
                return locale_charset();
            case code_page::UTF8:
                return "UTF-8";
            case code_page::GBK:
                return "GBK";
            case code_page::WCHAR:
                if constexpr (sizeof(wchar_t) == 2) {
                    return "UTF-16LE";
                } else {
                    return "UTF-32LE";
                }
            default:
                return "";
        }
    }

private:
    iconv_t cd;
};

std::string ws2s(std::wstring_view ws, code_page cp)
{
    iconv_wrapper conv(cp, code_page::WCHAR);
    std::string_view sv{(char*)ws.data(), ws.size() * sizeof(wchar_t)};
    return conv.convert<std::string>(sv, ws.size() * 6);
}

std::wstring s2ws(std::string_view s, code_page cp)
{
    iconv_wrapper conv(code_page::WCHAR, cp);
    return conv.convert<std::wstring>(s, s.size() * 4);
}

nonstd::unexpected_type<error> make_unexpected(std::string const& s)
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
