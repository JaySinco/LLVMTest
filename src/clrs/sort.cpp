#include "utils/base.h"
#include <catch2/catch.hpp>
#define SORT_SECTION(x)               \
    SECTION(#x)                       \
    {                                 \
        for (auto& [a, b]: samples) { \
            x(a.data(), a.size());    \
            REQUIRE(a == b);          \
        }                             \
    }

void insertion_sort(int* arr, int n)
{
    int bar = 1;
    while (bar < n) {
        int i = arr[bar];
        int j = bar - 1;
        while (j >= 0 && arr[j] > i) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = i;
        ++bar;
    }
}

void selection_sort(int* arr, int n)
{
    int bar = 0;
    while (bar < n - 1) {
        for (int i = bar + 1; i < n; ++i) {
            if (arr[i] < arr[bar]) {
                std::swap(arr[i], arr[bar]);
            }
        }
        ++bar;
    }
}

void merge_sort_merge(int* arr, int n, int half)
{
    int* copy = new int[n];
    std::memcpy(copy, arr, n * sizeof(int));
    int* ha = copy;
    int* hb = copy + half;
    int i = 0;
    while (i < n) {
        bool is_a = true;
        if (ha == copy + half || (hb != copy + n && *ha > *hb)) {
            is_a = false;
        }
        if (is_a) {
            arr[i] = *ha++;
        } else {
            arr[i] = *hb++;
        }
        ++i;
    }
    delete[] copy;
}

void merge_sort(int* arr, int n)
{
    if (n <= 1) {
        return;
    }
    int half = n / 2;
    int left = n - half;
    merge_sort(arr, half);
    merge_sort(arr + half, left);
    merge_sort_merge(arr, n, half);
}

TEST_CASE("sort")
{
    std::vector<std::pair<std::vector<int>, std::vector<int>>> samples = {
        {{}, {}},
        {{6}, {6}},
        {{5, 4, 2, 1, 2}, {1, 2, 2, 4, 5}},
        {{1, 2, 3, 4, 5, 6, 7, 8}, {1, 2, 3, 4, 5, 6, 7, 8}},
        {{10000, 1000, 100, 10, 1}, {1, 10, 100, 1000, 10000}},
        {{16, 4, 2, 0, 8}, {0, 2, 4, 8, 16}},
        {{1024, 64, 256, 65536, 8}, {8, 64, 256, 1024, 65536}},
        {{5, 1, 1, 1, 5}, {1, 1, 1, 5, 5}},
        {{1, 0, 0, 2, 1}, {0, 0, 1, 1, 2}},
        {{-1, 0, -10, -5, -3, 10}, {-10, -5, -3, -1, 0, 10}},
    };
    std::vector<int> origin(1024);
    for (int i = 0; i < 1024; ++i) {
        origin[i] = i;
    }
    auto rng = std::default_random_engine{};
    for (int i = 0; i < 100; ++i) {
        std::vector<int> copy(origin);
        std::shuffle(std::begin(copy), std::end(copy), rng);
        samples.push_back({copy, origin});
    }

    SORT_SECTION(insertion_sort);
    SORT_SECTION(selection_sort);
    SORT_SECTION(merge_sort);
}