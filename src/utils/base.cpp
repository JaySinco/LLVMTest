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
#include <fmt/chrono.h>
#include <spdlog/details/os.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace utils
{

class IconvWrapper
{
public:
    IconvWrapper(CodePage tocode, CodePage fromcode)
    {
        cd_ = iconv_open(code2str(tocode), code2str(fromcode));
    }

    ~IconvWrapper()
    {
        if (reinterpret_cast<iconv_t>(-1) == cd_) {
            return;
        }
        iconv_close(cd_);
    }

    size_t convert(char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
    {
        if (reinterpret_cast<iconv_t>(-1) == cd_) {
            return -1;
        }
        return iconv(cd_, inbuf, inbytesleft, outbuf, outbytesleft);
    }

    template <typename T = std::string>
    T convert(std::string_view in, size_t max_outbytes)
    {
        char* inbuf = const_cast<char*>(in.data());
        size_t inbytes = in.size();
        size_t outbytes = max_outbytes;
        std::unique_ptr<char> buf(new char[outbytes]{0});
        char* outbuf = buf.get();
        size_t ret = convert(&inbuf, &inbytes, &outbuf, &outbytes);
        if (static_cast<size_t>(-1) == ret) {
            return {};
        }
        using CharT = typename T::value_type;
        return T(reinterpret_cast<CharT*>(buf.get()), reinterpret_cast<CharT*>(outbuf));
    }

    static char const* code2str(CodePage cp)
    {
        switch (cp) {
            case CodePage::kLOCAL:
                setlocale(LC_ALL, "");
                return locale_charset();
            case CodePage::kUTF8:
                return "UTF-8";
            case CodePage::kGBK:
                return "GBK";
            case CodePage::kWCHAR:
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
    iconv_t cd_;
};

std::string ws2s(std::wstring_view ws, CodePage cp)
{
    IconvWrapper conv(cp, CodePage::kWCHAR);
    std::string_view sv{reinterpret_cast<char const*>(ws.data()), ws.size() * sizeof(wchar_t)};
    return conv.convert<std::string>(sv, ws.size() * 6);
}

std::wstring s2ws(std::string_view s, CodePage cp)
{
    IconvWrapper conv(CodePage::kWCHAR, cp);
    return conv.convert<std::wstring>(s, s.size() * 4);
}

nonstd::unexpected_type<Error> unexpected(std::string const& s)
{
    return nonstd::unexpected_type<Error>(s);
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

std::filesystem::path sourceRepo() { return std::filesystem::path(_SOURCE_REPO); }

void initLogger(std::string const& program, std::string const& log_dir)
{
    bool alsologtostderr = true;
    auto minloglevel = spdlog::level::debug;
    auto stderrlevel = spdlog::level::info;
    auto logbuflevel = spdlog::level::err;
    int logbufsecs = 30;
    int maxlogsize = 100;  // MB

    if (spdlog::get(program)) {
        return;
    }
    std::vector<spdlog::sink_ptr> sinks;
    if (!log_dir.empty()) {
        std::filesystem::path fpath(log_dir);
        std::string fname =
            FSTR("{}_{:%Y%m%d.%H%M%S}_{}.log", std::filesystem::path(program).stem().string(),
                 spdlog::details::os::localtime(), spdlog::details::os::pid());
        fpath /= fname;

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            fpath.string(), maxlogsize * 1024 * 1024, 10, true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("%L%m%d %H:%M:%S.%f %t %s:%#] %v");
        sinks.push_back(file_sink);
    }
    if (alsologtostderr) {
        auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        console_sink->set_level(stderrlevel);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
        sinks.push_back(console_sink);
    }

    auto logger = std::make_shared<spdlog::logger>(program, sinks.begin(), sinks.end());
    logger->set_level(minloglevel);
    logger->flush_on(logbuflevel);
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(logbufsecs));

    VLOG("### GIT HASH: {} ###", _GIT_HASH);
    VLOG("### GIT BRANCH: {} ###", _GIT_BRANCH);
    VLOG("### BUILD AT: {} {} ###", __DATE__, __TIME__);
}

}  // namespace utils
