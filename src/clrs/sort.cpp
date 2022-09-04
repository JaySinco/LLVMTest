#include "utils/base.h"
#include <catch2/catch.hpp>

void insertion_sort(int* arr, int n)
{
    int sorted = 0;
    while (sorted < n - 1) {
        int key = arr[sorted + 1];
        int i = sorted;
        while (i >= 0 && arr[i] > key) {
            arr[i + 1] = arr[i];
            --i;
        }
        arr[i + 1] = key;
        ++sorted;
    }
}

TEST_CASE("sort")
{
    std::vector<std::pair<std::vector<int>, std::vector<int>>> samples = {
        {{}, {}},
        {{6}, {6}},
        {{5, 4, 2, 1, 2}, {1, 2, 2, 4, 5}},
    };
    SECTION("insertion_sort")
    {
        for (auto& [a, b]: samples) {
            insertion_sort(a.data(), a.size());
            CHECK(a == b);
        }
    }
}