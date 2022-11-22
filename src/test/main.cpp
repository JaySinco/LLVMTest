#include "utils/logging.h"

int main(int argc, char** argv)
{
    utils::initLogger(argv[0]);
    std::string world = "world";
    ILOG("hello, {}", world);
}
