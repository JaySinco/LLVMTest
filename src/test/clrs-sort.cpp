#include "utils/base.h"
#include <fmt/ranges.h>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

std::default_random_engine g_random_engine;

void insertionSort(int* arr, int n)
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

void selectionSort(int* arr, int n)
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

void mergeSortMerge(int* arr, int n, int half)
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

void mergeSort(int* arr, int n)
{
    if (n <= 1) {
        return;
    }
    int half = n / 2;
    int left = n - half;
    mergeSort(arr, half);
    mergeSort(arr + half, left);
    mergeSortMerge(arr, n, half);
}

int calcInversionsMerge(int* arr, int n, int half)
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

int calcInversions(int* arr, int n)
{
    if (n <= 1) {
        return 0;
    }
    int half = n / 2;
    int left = n - half;
    int i1 = calcInversions(arr, half);
    int i2 = calcInversions(arr + half, left);
    int i3 = calcInversionsMerge(arr, n, half);
    return i1 + i2 + i3;
}

void bubbleSort(int* arr, int n)
{
    for (int i = 0; i < n - 1; ++i) {
        for (int j = n - 1; j > i; --j) {
            if (arr[j] < arr[j - 1]) {
                std::swap(arr[j], arr[j - 1]);
            }
        }
    }
}

void quickSort(int* arr, int n)
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
    quickSort(arr, p);
    quickSort(arr + p + 1, n - p - 1);
}

class Heap
{
public:
    using Comp = std::function<bool(int, int)>;

    Heap(int* arr, int n, Comp op = std::greater<int>()): arr_(arr), n_(n), op_(op) {}

    void resize(int dn) { this->n_ += dn; }

    static int parent(int i) { return (i + 1) / 2 - 1; }

    static int leftChild(int i) { return (i + 1) * 2 - 1; }

    static int rightChild(int i) { return leftChild(i) + 1; }

    bool isLeaf(int i) const { return leftChild(i) >= n_; }

    void pullDown(int i)
    {
        while (!isLeaf(i)) {
            int lc = leftChild(i);
            int rc = rightChild(i);
            int idx = i;
            if (lc < n_ && op_(arr_[lc], arr_[idx])) {
                idx = lc;
            }
            if (rc < n_ && op_(arr_[rc], arr_[idx])) {
                idx = rc;
            }
            if (idx == i) {
                break;
            }
            std::swap(arr_[idx], arr_[i]);
            i = idx;
        }
    }

    void liftUp(int i)
    {
        while (i > 0) {
            int p = parent(i);
            if (op_(arr_[i], arr_[p])) {
                std::swap(arr_[i], arr_[p]);
                i = p;
            } else {
                break;
            }
        }
    }

    void buildHeap()
    {
        for (int i = n_ / 2 - 1; i >= 0; --i) {
            pullDown(i);
        }
    }

private:
    int* arr_;
    int n_;
    Comp op_;
};

void heapSort(int* arr, int n)
{
    Heap hp(arr, n);
    hp.buildHeap();
    for (int i = n - 1; i > 0; --i) {
        std::swap(arr[0], arr[i]);
        hp.resize(-1);
        hp.pullDown(0);
    }
}

void countSort(int* arr, int n)
{
    int const k = 10240 + 1;
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
    explicit PriorityQueue(std::vector<int> const& data, Heap::Comp op = std::greater<int>())
        : vec_(data), op_(op)
    {
        Heap hp(vec_.data(), vec_.size(), op);
        hp.buildHeap();
    }

    int top()
    {
        if (!vec_.empty()) {
            return vec_[0];
        }
        THROW_("queue is empty!");
    }

    void popTop()
    {
        std::swap(vec_[0], vec_.back());
        vec_.pop_back();
        Heap hp(vec_.data(), vec_.size(), op_);
        hp.pullDown(0);
    }

    void change(int i, int v)
    {
        int old = vec_.at(i);
        vec_[i] = v;
        Heap hp(vec_.data(), vec_.size(), op_);
        if (!op_(v, old)) {
            hp.pullDown(i);
        } else if (op_(v, old)) {
            hp.liftUp(i);
        }
    }

    void insert(int x)
    {
        vec_.push_back(x);
        Heap hp(vec_.data(), vec_.size(), op_);
        hp.liftUp(vec_.size() - 1);
    }

    int size() { return vec_.size(); }

private:
    std::vector<int> vec_;
    Heap::Comp op_;
};

int selectIthOrder(int* arr, int n, int i)
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
        return selectIthOrder(arr, p, i);
    } else {
        return selectIthOrder(arr + p + 1, n - p - 1, i - (p + 1));
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
        int i = selectIthOrder(a.data(), a.size(), b);
        REQUIRE(i == c);
    }
    std::vector<int> vec(1024);
    for (int i = 0; i < 1024; ++i) {
        vec[i] = i + 1;
    }
    for (int i = 1; i <= 1024; ++i) {
        std::shuffle(std::begin(vec), std::end(vec), g_random_engine);
        int j = selectIthOrder(vec.data(), vec.size(), i);
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
        queue.popTop();
        CHECK(queue.top() == 27);
        queue.popTop();
        queue.insert(199);
        CHECK(queue.top() == 199);
        int last = 200;
        while (queue.size() > 0) {
            CHECK(last > queue.top());
            last = queue.top();
            queue.popTop();
        }
    }
    SECTION("min_heap")
    {
        PriorityQueue queue(data, std::less<int>());
        CHECK(queue.top() == 1);
        queue.popTop();
        CHECK(queue.top() == 2);
        queue.popTop();
        queue.insert(199);
        CHECK(queue.top() == 3);
        int last = 2;
        while (queue.size() > 0) {
            CHECK(last < queue.top());
            last = queue.top();
            queue.popTop();
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
        int i = calcInversions(a.data(), a.size());
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
        samples.emplace_back(copy, origin);
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

    SORT_SECTION(insertionSort);
    SORT_SECTION(selectionSort);
    SORT_SECTION(mergeSort);
    SORT_SECTION(bubbleSort);
    SORT_SECTION(quickSort);
    SORT_SECTION(heapSort);
    SORT_SECTION(countSort);
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

    SORT_BENCHMARK(insertionSort);
    SORT_BENCHMARK(selectionSort);
    SORT_BENCHMARK(mergeSort);
    SORT_BENCHMARK(bubbleSort);
    SORT_BENCHMARK(quickSort);
    SORT_BENCHMARK(heapSort);
    SORT_BENCHMARK(countSort);
}

int main(int argc, char* argv[])
{
    Catch::Session session;
    auto& config = session.configData();
    config.benchmarkSamples = 10;
    config.benchmarkNoAnalysis = true;
    config.shouldDebugBreak = true;
    int return_code = session.applyCommandLine(argc, argv);
    if (return_code != 0) {
        return return_code;
    }
    int num_failed = session.run();
    return num_failed;
}