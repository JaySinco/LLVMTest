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

class Heap
{
public:
    using Comp = std::function<bool(int, int)>;
    Heap(int* arr, int n, Comp op = std::greater<int>()): arr(arr), n(n), op(std::move(op)) {}
    void resize(int dn) { this->n += dn; }
    int parent(int i) { return (i + 1) / 2 - 1; }
    int left_child(int i) { return (i + 1) * 2 - 1; }
    int right_child(int i) { return left_child(i) + 1; }
    bool is_leaf(int i) { return left_child(i) >= n; }
    void pull_down(int i)
    {
        while (!is_leaf(i)) {
            int lc = left_child(i);
            int rc = right_child(i);
            int idx = i;
            if (lc < n && op(arr[lc], arr[idx])) idx = lc;
            if (rc < n && op(arr[rc], arr[idx])) idx = rc;
            if (idx == i) break;
            std::swap(arr[idx], arr[i]);
            i = idx;
        }
    }
    void build_heap()
    {
        for (int i = n / 2 - 1; i >= 0; --i) {
            pull_down(i);
        }
    }

private:
    int* arr;
    int n;
    Comp op;
};

void heap_sort(int* arr, int n)
{
    Heap hp(arr, n);
    hp.build_heap();
    for (int i = n - 1; i > 0; --i) {
        std::swap(arr[0], arr[i]);
        hp.resize(-1);
        hp.pull_down(0);
    }
}

std::default_random_engine g_random_engine;

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
    for (int i = 0; i < 100; ++i) {
        std::vector<int> copy(origin);
        std::shuffle(std::begin(copy), std::end(copy), g_random_engine);
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
    SORT_SECTION(heap_sort);
}

TEST_CASE("sort_benchmark", "[benchmark]")
{
    std::vector<int> bcArr(10240);
    for (int i = 0; i < 10240; ++i) {
        bcArr[i] = i;
    }
    std::shuffle(std::begin(bcArr), std::end(bcArr), g_random_engine);

#define SORT_BENCHMARK(x) \
    BENCHMARK(#x) { return x(bcArr.data(), bcArr.size()); };

    SORT_BENCHMARK(insertion_sort);
    SORT_BENCHMARK(merge_sort);
    SORT_BENCHMARK(quick_sort);
    SORT_BENCHMARK(heap_sort);
}