#include "utils.h"
#include <fstream>
#include <sstream>
#include <codecvt>
#include <unistd.h>
#include <limits.h>

namespace utils
{
std::string ws2s(const std::wstring& ws)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(ws);
}

std::wstring s2ws(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s);
}

nonstd::unexpected_type<error> make_unexpected(const std::string& s)
{
    return nonstd::unexpected_type<error>(s);
}

expected<std::string> readFile(const std::string& path)
{
    std::ifstream in_file(path);
    if (!in_file) {
        return make_unexpected("failed to open file");
    }
    std::stringstream ss;
    ss << in_file.rdbuf();
    return ss.str();
}

std::string getExeDir()
{
    char cwd[PATH_MAX] = {0};
    getcwd(cwd, sizeof(cwd));
    return cwd;
}

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

std::string base64_encode(const unsigned char* buf, unsigned int bufLen)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (bufLen--) {
        char_array_3[i++] = *(buf++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];

        while ((i++ < 3)) ret += '=';
    }

    return ret;
}

std::vector<unsigned char> base64_decode(const std::string& encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) char_array_4[j] = 0;

        for (j = 0; j < 4; j++) char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

class WordDistance
{
public:
    static int calc(const std::string& s1, const std::string& s2, bool logStep = false)
    {
        int s1n = s1.size();
        int s2n = s2.size();
        Record record(s1n + 1, std::vector<Result>(s2n + 1));
        for (int i = 0; i <= s1n; ++i) {
            record[i][0] = Result{i, DELETE_};
        }
        for (int j = 0; j <= s2n; ++j) {
            record[0][j] = Result{j, ADD};
        }
        for (int i = 1; i <= s1n; ++i) {
            for (int j = 1; j <= s2n; ++j) {
                Result r1 = {record[i][j - 1].n + 1, ADD};
                Result r2 = {record[i - 1][j].n + 1, DELETE_};
                Result r3 = {record[i - 1][j - 1].n + (s1[i - 1] == s2[j - 1] ? 0 : 1), CHANGE};
                std::vector<Result> buf{r1, r2, r3};
                auto it = std::min_element(buf.begin(), buf.end(),
                                           [](Result& a, Result& b) { return a.n < b.n; });
                record[i][j] = *it;
            }
        }
        if (logStep) {
            std::vector<Step> steps;
            doLogStep(s1, s2, s1n, s2n, record, steps);
            int count = 1;
            const char* templ = "{:<3d} {:{}s}";
            int sep = std::max(s1n, s2n) + 2;
            std::cout << fmt::format(templ, count++, s1, sep);
            for (auto& s: steps) {
                std::cout << s.desc << std::endl << fmt::format(templ, count++, s.s, sep);
            }
            std::cout << std::endl;
        }
        return record[s1n][s2n].n;
    }

private:
    enum OpType
    {
        ADD,
        DELETE_,
        CHANGE,
    };

    struct Result
    {
        int n;
        OpType op;
    };

    struct Step
    {
        std::string s;
        std::string desc;
    };

    using Record = std::vector<std::vector<Result>>;

    static void doLogStep(const std::string& s1, const std::string& s2, int i, int j,
                          const Record& record, std::vector<Step>& steps)
    {
        if (i == 0 && j == 0) {
            return;
        }
        const char* templ = "{:6s} {} {}";
        switch (record[i][j].op) {
            case ADD:
                doLogStep(s1, s2, i, j - 1, record, steps);
                steps.push_back({s2.substr(0, j), fmt::format(templ, "ADD", j, s2[j - 1])});
                break;
            case DELETE_:
                steps.push_back({s1.substr(0, i - 1), fmt::format(templ, "DELETE", i, s1[i - 1])});
                doLogStep(s1, s2, i - 1, j, record, steps);
                break;
            case CHANGE: {
                std::vector<Step> buf;
                doLogStep(s1, s2, i - 1, j - 1, record, buf);
                for (auto s: buf) {
                    s.s += s1[i - 1];
                    steps.push_back(s);
                }
                if (s1[i - 1] != s2[j - 1]) {
                    steps.push_back({s2.substr(0, j),
                                     fmt::format(templ, "CHANGE", j,
                                                 fmt::format("{} -> {}", s1[i - 1], s2[j - 1]))});
                }
                break;
            }
            default:
                break;
        }
    }
};

int word_distance(const std::string& s1, const std::string& s2, bool logStep)
{
    return WordDistance::calc(s1, s2, logStep);
}

}  // namespace utils
