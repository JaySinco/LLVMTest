#include "utils/base.h"
#include <ranges>

int main(int argc, char** argv)
{
    auto const ints = {0, 1, 2, 3, 4, 5};
    auto even = [](int i) { return 0 == i % 2; };
    auto square = [](int i) { return i * i; };
    for (int i: ints | std::views::filter(even) | std::views::transform(square)) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}
