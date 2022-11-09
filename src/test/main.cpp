#include "utils/base.h"

int main(int argc, char** argv)
{
    std::string world = "world";
    spdlog::info("hello, {}", world);
}
