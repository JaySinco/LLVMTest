#include "utils/base.h"
#include <range/v3/all.hpp>

int main(int argc, char** argv)
{
    utils::initLogger(argv[0]);
    auto const ints = {0, 1, 2, 3, 4, 5};
    auto even = [](int i) { return 0 == i % 2; };
    auto square = [](int i) { return i * i; };
    for (int i: ints | ranges::views::filter(even) | ranges::views::transform(square)) {
        std::cout << i << ' ';
    }
    std::cout << '\n';

    auto trim_front = ranges::views::drop_while(::isspace);
    auto trim_back = ranges::views::reverse | trim_front | ranges::views::reverse;
    auto trim = trim_front | trim_back;
    std::string s = "  ABC D  ";
    std::cout << (s | trim | ranges::to<std::string>) << std::endl;
}
