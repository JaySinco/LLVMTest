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
    auto ans = [&](int i, int j) -> std::string {
        auto la = [&](int i, int j, auto& la_) -> std::string {
            std::string s = "";
            if (i == j) {
                s = fmt::format("A{}", i);
            } else {
                auto k = dp[i][j].i;
                s = fmt::format("({}{})", la_(i, k, la_), la_(k + 1, j, la_));
            }
            return s;
        };
        return la(i, j, la);
    };
    spdlog::debug("ans={}", ans(1, n));
    return dp[1][n].v;
}

std::string longest_common_subseq(std::string_view sv1, std::string_view sv2)
{
    struct Record
    {
        int v;
        char a;
    };
    int na = sv1.size();
    int nb = sv2.size();
    std::vector<std::vector<Record>> dp(na + 1, std::vector<Record>(nb + 1, {0, ' '}));
    for (int i = 1; i <= na; ++i) {
        for (int j = 1; j <= nb; ++j) {
            if (sv1[i - 1] == sv2[j - 1]) {
                dp[i][j].v = dp[i - 1][j - 1].v + 1;
                dp[i][j].a = '*';
            } else if (dp[i][j - 1].v > dp[i - 1][j].v) {
                dp[i][j].v = dp[i][j - 1].v;
                dp[i][j].a = '>';
            } else {
                dp[i][j].v = dp[i - 1][j].v;
                dp[i][j].a = '<';
            }
        }
    }
    std::string ans;
    auto build_ans = [&](int i, int j) {
        auto la = [&](int i, int j, auto& la_) {
            if (i == 0 || j == 0) return;
            if (dp[i][j].a == '*') {
                la_(i - 1, j - 1, la_);
                ans.push_back(sv1[i - 1]);
            } else if (dp[i][j].a == '>') {
                la_(i, j - 1, la_);
            } else if (dp[i][j].a == '<') {
                la_(i - 1, j, la_);
            }
        };
        return la(i, j, la);
    };
    build_ans(na, nb);
    return ans;
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

    SECTION("longest_common_subseq")
    {
        std::vector<std::tuple<std::string, std::string, std::string>> samples = {
            {"ABCBDAB", "BDCABA", "BCBA"},
        };
        for (auto& [a, b, c]: samples) {
            INFO(fmt::format("a={}, b={}", a, b));
            auto r = longest_common_subseq(a, b);
            CHECK(r == c);
        }
    }
}
