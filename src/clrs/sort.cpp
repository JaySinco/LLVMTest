#include "utils/base.h"
#include <fmt/ranges.h>
#include <catch2/catch.hpp>

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

int calc_inversions_merge(int* arr, int n, int half)
{
    int inv = 0;
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
            inv += (copy + half - ha);
        }
        ++i;
    }
    delete[] copy;
    return inv;
}

int calc_inversions(int* arr, int n)
{
    if (n <= 1) {
        return 0;
    }
    int half = n / 2;
    int left = n - half;
    int i1 = calc_inversions(arr, half);
    int i2 = calc_inversions(arr + half, left);
    int i3 = calc_inversions_merge(arr, n, half);
    return i1 + i2 + i3;
}

void bubble_sort(int* arr, int n)
{
    for (int i = 0; i < n - 1; ++i) {
        for (int j = n - 1; j > i; --j) {
            if (arr[j] < arr[j - 1]) {
                std::swap(arr[j], arr[j - 1]);
            }
        }
    }
}

void quick_sort(int* arr, int n)
{
    if (n <= 1) {
        return;
    }
    int piv = 0;
    int i = piv + 1;
    while (i < n) {
        if (arr[i] < arr[piv]) {
            if (piv + 1 == i) {
                std::swap(arr[piv], arr[i]);
            } else {
                std::swap(arr[piv], arr[piv + 1]);
                std::swap(arr[piv], arr[i]);
            }
            ++piv;
        }
        ++i;
    }
    quick_sort(arr, piv);
    quick_sort(arr + piv + 1, n - piv - 1);
}

TEST_CASE("calc_inversions")
{
    std::vector<std::pair<std::vector<int>, int>> samples = {
        {{}, 0},
        {{2}, 0},
        {{3, 2}, 1},
        {{3, 2, 1}, 3},
        {{5, 2, 4, 1}, 5},
        {{2, 3, 8, 6, 1}, 5},
        {{1, 2, 3, 4, 5, 6, 7, 8}, 0},
    };
    for (auto& [a, b]: samples) {
        CAPTURE(a);
        int i = calc_inversions(a.data(), a.size());
        REQUIRE(i == b);
    }
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

#define SORT_SECTION(x)                        \
    SECTION(#x)                                \
    {                                          \
        for (auto& [a, b]: samples) {          \
            INFO(fmt::format("arr := {}", a)); \
            x(a.data(), a.size());             \
            REQUIRE(a == b);                   \
        }                                      \
    }

    SORT_SECTION(insertion_sort);
    SORT_SECTION(selection_sort);
    SORT_SECTION(merge_sort);
    SORT_SECTION(bubble_sort);
    SORT_SECTION(quick_sort);
}