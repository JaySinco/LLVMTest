#pragma once
#include "error.h"
#include <filesystem>
#define CURR_DIRNAME (std::filesystem::path(__FILE__).parent_path())
#define CURR_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define CURR_RESDIR (CURR_DIRNAME / "res")

namespace utils
{

std::filesystem::path sourceRepo();
std::wstring getExeDir();
std::wstring defaultLoggingDir();
Expected<std::string> readFile(std::wstring_view path);

}  // namespace utils
