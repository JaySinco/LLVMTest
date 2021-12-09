#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "utils.h"
#include <fstream>
#include <sstream>

namespace utils
{
std::string ws2s(const std::wstring& ws, bool u8_instead_of_ansi)
{
    UINT page = u8_instead_of_ansi ? CP_UTF8 : CP_ACP;
    int len = WideCharToMultiByte(page, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    auto buf = new char[len]{0};
    if (buf == nullptr) return "";
    WideCharToMultiByte(page, 0, ws.c_str(), -1, buf, len, nullptr, nullptr);
    std::string s = buf;
    delete[] buf;
    return s;
}

std::wstring s2ws(const std::string& s, bool u8_instead_of_ansi)
{
    UINT page = u8_instead_of_ansi ? CP_UTF8 : CP_ACP;
    int len = MultiByteToWideChar(page, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    auto buf = new wchar_t[len]{0};
    if (buf == nullptr) return L"";
    MultiByteToWideChar(page, 0, s.c_str(), -1, buf, len);
    std::wstring ws = buf;
    delete[] buf;
    return ws;
}

nonstd::unexpected_type<error> make_unexpected(const std::string& s)
{
    return nonstd::unexpected_type<error>(s);
}

expected<std::string> readFile(const std::wstring& path)
{
    std::ifstream in_file(path);
    if (!in_file) {
        return make_unexpected("failed to open file");
    }
    std::stringstream ss;
    ss << in_file.rdbuf();
    return ss.str();
}

std::wstring getExeDir()
{
    wchar_t buf[MAX_PATH + 1] = {0};
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    (wcsrchr(buf, L'\\'))[0] = 0;
    return buf;
}

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(BYTE c) { return (isalnum(c) || (c == '+') || (c == '/')); }

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

}  // namespace utils
