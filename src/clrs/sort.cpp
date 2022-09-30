#include "utils/base.h"
#include <fmt/ranges.h>
#include <catch2/catch.hpp>

std::default_random_engine g_random_engine;

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
    std::uniform_int_distribution<int> dis(0, n - 1);
    std::swap(arr[dis(g_random_engine)], arr[n - 1]);
    int x = arr[n - 1];
    int p = -1;
    for (int i = 0; i < n - 1; ++i) {
        if (arr[i] < x) {
            ++p;
            std::swap(arr[p], arr[i]);
        }
    }
    ++p;
    std::swap(arr[n - 1], arr[p]);
    quick_sort(arr, p);
    quick_sort(arr + p + 1, n - p - 1);
}

class Heap
{
public:
    using Comp = std::function<bool(int, int)>;
    Heap(int* arr, int n, Comp op = std::greater<int>()): arr(arr), n(n), op(op) {}
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
    void lift_up(int i)
    {
        while (i > 0) {
            int p = parent(i);
            if (op(arr[i], arr[p])) {
                std::swap(arr[i], arr[p]);
                i = p;
            } else {
                break;
            }
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

void count_sort(int* arr, int n)
{
    constexpr int k = 10240 + 1;
    std::vector<int> count(k, 0);
    for (int i = 0; i < n; ++i) {
        assert(arr[i] > 0 && arr[i] < k);
        ++count[arr[i]];
    }
    for (int i = 1; i < k; ++i) {
        count[i] += count[i - 1];
    }
    int j = n - 1;
    while (j >= 0) {
        if (arr[j] < 0) {
            arr[j] *= -1;
            --j;
            continue;
        }
        int i = count[arr[j]] - 1;
        assert(i <= j);
        --count[arr[j]];
        if (i == j) {
            --j;
            continue;
        }
        std::swap(arr[j], arr[i]);
        arr[i] *= -1;
    }
}

class PriorityQueue
{
public:
    PriorityQueue(std::vector<int> const& data, Heap::Comp op = std::greater<int>())
        : vec(data), op(op)
    {
        Heap hp(vec.data(), vec.size(), op);
        hp.build_heap();
    }
    int top()
    {
        if (!vec.empty()) {
            return vec[0];
        }
        throw std::runtime_error("queue is empty!");
    }
    void pop_top()
    {
        std::swap(vec[0], vec.back());
        vec.pop_back();
        Heap hp(vec.data(), vec.size(), op);
        hp.pull_down(0);
    }
    void change(int i, int v)
    {
        int old = vec.at(i);
        vec[i] = v;
        Heap hp(vec.data(), vec.size(), op);
        if (!op(v, old)) {
            hp.pull_down(i);
        } else if (op(v, old)) {
            hp.lift_up(i);
        }
    }
    void insert(int x)
    {
        vec.push_back(x);
        Heap hp(vec.data(), vec.size(), op);
        hp.lift_up(vec.size() - 1);
    }
    int size() { return vec.size(); }

private:
    std::vector<int> vec;
    Heap::Comp op;
};

int select_ith_order(int* arr, int n, int i)
{
    assert(i >= 1 && i <= n);
    if (n <= 1) {
        return arr[0];
    }
    std::uniform_int_distribution<int> dis(0, n - 1);
    std::swap(arr[dis(g_random_engine)], arr[n - 1]);
    int k = arr[n - 1];
    int p = -1;
    for (int i = 0; i < n - 1; ++i) {
        if (arr[i] < k) {
            ++p;
            std::swap(arr[p], arr[i]);
        }
    }
    ++p;
    std::swap(arr[p], arr[n - 1]);
    if (p + 1 == i) {
        return arr[p];
    } else if (p + 1 > i) {
        return select_ith_order(arr, p, i);
    } else {
        return select_ith_order(arr + p + 1, n - p - 1, i - (p + 1));
    }
}

TEST_CASE("select_ith_order")
{
    std::vector<std::tuple<std::vector<int>, int, int>> samples = {
        {{2}, 1, 2},
        {{3, 2}, 1, 2},
        {{3, 2}, 2, 3},
        {{3, 2, 1}, 3, 3},
        {{5, 2, 4, 1}, 2, 2},
        {{5, 2, 4, 1}, 4, 5},
        {{2, 3, 8, 6, 1}, 1, 1},
        {{2, 3, 8, 6, 1}, 5, 8},
        {{2, 3, 8, 6, 1}, 4, 6},
    };
    for (auto& [a, b, c]: samples) {
        INFO(fmt::format("arr={}, i={}", a, b));
        int i = select_ith_order(a.data(), a.size(), b);
        REQUIRE(i == c);
    }
    std::vector<int> vec(1024);
    for (int i = 0; i < 1024; ++i) {
        vec[i] = i + 1;
    }
    for (int i = 1; i <= 1024; ++i) {
        std::shuffle(std::begin(vec), std::end(vec), g_random_engine);
        int j = select_ith_order(vec.data(), vec.size(), i);
        REQUIRE(i == j);
    }
}

TEST_CASE("priority_queue")
{
    std::vector<int> data{5, 6, 27, 1, 2, 3, 9, 10, 20, 23, 49};
    SECTION("max_heap")
    {
        PriorityQueue queue(data, std::greater<int>());
        CHECK(queue.top() == 49);
        queue.pop_top();
        CHECK(queue.top() == 27);
        queue.pop_top();
        queue.insert(199);
        CHECK(queue.top() == 199);
        int last = 200;
        while (queue.size() > 0) {
            CHECK(last > queue.top());
            last = queue.top();
            queue.pop_top();
        }
    }
    SECTION("min_heap")
    {
        PriorityQueue queue(data, std::less<int>());
        CHECK(queue.top() == 1);
        queue.pop_top();
        CHECK(queue.top() == 2);
        queue.pop_top();
        queue.insert(199);
        CHECK(queue.top() == 3);
        int last = 2;
        while (queue.size() > 0) {
            CHECK(last < queue.top());
            last = queue.top();
            queue.pop_top();
        }
    }
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
        {{16, 4, 2, 1, 8}, {1, 2, 4, 8, 16}},
        {{1024, 64, 256, 6553, 8}, {8, 64, 256, 1024, 6553}},
        {{5, 1, 1, 1, 5}, {1, 1, 1, 5, 5}},
        {{1, 2, 1, 2, 1}, {1, 1, 1, 2, 2}},
    };
    std::vector<int> origin(1024);
    for (int i = 0; i < 1024; ++i) {
        origin[i] = i + 1;
    }
    for (int i = 0; i < 100; ++i) {
        std::vector<int> copy(origin);
        std::shuffle(std::begin(copy), std::end(copy), g_random_engine);
        samples.push_back({copy, origin});
    }

#define SORT_SECTION(x)                     \
    SECTION(#x)                             \
    {                                       \
        for (auto& [a, b]: samples) {       \
            INFO(fmt::format("arr={}", a)); \
            x(a.data(), a.size());          \
            REQUIRE(a == b);                \
        }                                   \
    }

    SORT_SECTION(insertion_sort);
    SORT_SECTION(selection_sort);
    SORT_SECTION(merge_sort);
    SORT_SECTION(bubble_sort);
    SORT_SECTION(quick_sort);
    SORT_SECTION(heap_sort);
    SORT_SECTION(count_sort);
}

TEST_CASE("sort_benchmark", "[benchmark]")
{
#define SORT_BENCHMARK(x)                                              \
    BENCHMARK_ADVANCED(#x)(Catch::Benchmark::Chronometer meter)        \
    {                                                                  \
        std::vector<int> vec(10240);                                   \
        for (int i = 0; i < 10240; ++i) {                              \
            vec[i] = i + 1;                                            \
        }                                                              \
        std::shuffle(std::begin(vec), std::end(vec), g_random_engine); \
        meter.measure([&] { return x(vec.data(), vec.size()); });      \
    }

    SORT_BENCHMARK(insertion_sort);
    SORT_BENCHMARK(selection_sort);
    SORT_BENCHMARK(merge_sort);
    SORT_BENCHMARK(bubble_sort);
    SORT_BENCHMARK(quick_sort);
    SORT_BENCHMARK(heap_sort);
    SORT_BENCHMARK(count_sort);
}
