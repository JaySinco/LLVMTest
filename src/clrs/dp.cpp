#include "utils/base.h"
#include <fmt/ranges.h>
#include <range/v3/all.hpp>
#include <functional>
#include <stack>
#include <catch2/catch.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

std::vector<int> rod_cutting(int* prices, int n, int cut_cost = 0)
{
    struct Record
    {
        int v;
        int i;
    };
    std::vector<Record> dp(n + 1, {0, 0});
    for (int j = 1; j <= n; ++j) {
        for (int i = 1; i <= j; ++i) {
            int a = dp[j - i].v + prices[i - 1];
            if (i < j) {
                a -= cut_cost;
            }
            if (a > dp[j].v) {
                dp[j].v = a;
                dp[j].i = i;
            }
        }
    }
    std::vector<int> ans;
    int m = n;
    while (m > 0) {
        ans.push_back(dp[m].i);
        m = m - ans.back();
    }
    ranges::reverse(ans);
    return ans;
}

cpp_int matrix_chain_multi(int* dims, int n)
{
    struct Record
    {
        cpp_int v;
        int i;
    };
    std::vector<std::vector<Record>> dp(n + 1, std::vector<Record>(n + 1, {0, 0}));
    for (int k = 1; k < n; ++k) {
        for (int i = 1; i + k <= n; ++i) {
            int j = i + k;
            for (int m = i; m < j; ++m) {
                auto v =
                    dp[i][m].v + dp[m + 1][j].v + dims[i - 1] * dims[m + 1 - 1] * dims[j + 1 - 1];
                if (m == i || v < dp[i][j].v) {
                    dp[i][j].v = v;
                    dp[i][j].i = m;
                }
            }
        }
    }
    std::function<std::string(int, int)> printFunc = [&](int i, int j) -> std::string {
        std::string s = "";
        if (i == j) {
            s = fmt::format("A{}", i);
        } else {
            auto k = dp[i][j].i;
            s = fmt::format("({}{})", printFunc(i, k), printFunc(k + 1, j));
        }
        return s;
    };
    // spdlog::info(printFunc(1, n));
    return dp[1][n].v;
}

TEST_CASE("dynamic_programming")
{
    SECTION("rod_cutting")
    {
        std::vector<std::pair<std::vector<int>, std::vector<int>>> samples = {
            {{1, 5, 8, 9}, {2, 2}},
            {{1, 5, 8, 9, 10}, {3, 2}},
            {{1, 5, 8, 9, 10, 17, 17}, {6, 1}},
            {{1, 5, 8, 9, 10, 17, 17, 20, 24}, {6, 3}},
            {{1, 5, 8, 9, 10, 17, 17, 20, 24, 30}, {10}},
        };
        for (auto& [a, b]: samples) {
            INFO(fmt::format("arr={}", a));
            auto c = rod_cutting(a.data(), a.size());
            CHECK(ranges::accumulate(c, 0) == a.size());
            CHECK(c == b);
        }
    }

    SECTION("matrix_chain_multi")
    {
        std::vector<std::pair<std::vector<int>, cpp_int>> samples = {
            {{30, 35, 15, 5, 10, 20, 25}, 15125},
        };
        for (auto& [a, b]: samples) {
            INFO(fmt::format("arr={}", a));
            auto c = matrix_chain_multi(a.data(), a.size() - 1);
            CHECK(c == b);
        }
    }
}
