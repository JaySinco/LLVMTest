#include "utils/base.h"
#include <fmt/ranges.h>
#include <range/v3/all.hpp>
#include <functional>
#include <stack>
#include <iterator>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

std::vector<int> rodCutting(int const* prices, int n, int cut_cost = 0)
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

cpp_int matrixChainMulti(int const* dims, int n)
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
        auto la = [&](int i, int j, auto& lai) -> std::string {
            std::string s = "";
            if (i == j) {
                s = FSTR("A{}", i);
            } else {
                auto k = dp[i][j].i;
                s = FSTR("({}{})", lai(i, k, lai), lai(k + 1, j, lai));
            }
            return s;
        };
        return la(i, j, la);
    };
    VLOG("ans={}", ans(1, n));
    return dp[1][n].v;
}

std::string longestCommonSubseq(std::string_view sv1, std::string_view sv2)
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
        auto la = [&](int i, int j, auto& lai) {
            if (i == 0 || j == 0) {
                return;
            }
            if (dp[i][j].a == '*') {
                lai(i - 1, j - 1, lai);
                ans.push_back(sv1[i - 1]);
            } else if (dp[i][j].a == '>') {
                lai(i, j - 1, lai);
            } else if (dp[i][j].a == '<') {
                lai(i - 1, j, lai);
            }
        };
        return la(i, j, la);
    };
    build_ans(na, nb);
    return ans;
}

std::vector<int> longestIncreasingSubseq(int* arr, int n)
{
    struct Record
    {
        int v;
        int i;
    };

    std::vector<Record> dp(n);
    for (int i = 0; i < n; ++i) {
        dp[i].v = 1;
        dp[i].i = -1;
        for (int k = 0; k < i; ++k) {
            if (arr[k] < arr[i] && dp[k].v + 1 > dp[i].v) {
                dp[i].v = dp[k].v + 1;
                dp[i].i = k;
            }
        }
    }
    std::vector<int> ans;
    auto mi =
        ranges::max_element(dp, [](Record const& r1, Record const& r2) { return r1.v < r2.v; });
    int j = std::distance(dp.begin(), mi);
    while (j >= 0) {
        ans.push_back(arr[j]);
        j = dp[j].i;
    }
    ranges::reverse(ans);
    return ans;
}

std::string longestPalindromeSubseq(std::string const& s)
{
    struct Record
    {
        int v;
        char c;
    };

    int n = s.size();
    std::vector<std::vector<Record>> dp(n, std::vector<Record>(n, {0, 0}));
    for (int i = 0; i < n; ++i) {
        dp[i][i] = {1, '*'};
    }
    for (int k = 1; k < n; ++k) {
        for (int i = 0; i + k < n; ++i) {
            int j = i + k;
            if (s[i] == s[j]) {
                dp[i][j].v = (k == 1 ? 0 : dp[i + 1][j - 1].v) + 2;
                dp[i][j].c = '-';
            } else if (dp[i][j - 1].v > dp[i + 1][j].v) {
                dp[i][j].v = dp[i][j - 1].v;
                dp[i][j].c = '<';
            } else {
                dp[i][j].v = dp[i + 1][j].v;
                dp[i][j].c = '>';
            }
        }
    }
    std::string ans;
    auto build_ans = [&](int i, int j) {
        auto la = [&](int i, int j, auto& lai) {
            if (i > j) {
                return;
            } else if (dp[i][j].c == '*') {
                ans.push_back(s[i]);
            } else if (dp[i][j].c == '-') {
                ans.push_back(s[i]);
                lai(i + 1, j - 1, lai);
                ans.push_back(s[j]);
            } else if (dp[i][j].c == '<') {
                lai(i, j - 1, lai);
            } else if (dp[i][j].c == '>') {
                lai(i + 1, j, lai);
            }
        };
        return la(i, j, la);
    };
    build_ans(0, n - 1);
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
            INFO(FSTR("arr={}", a));
            auto c = rodCutting(a.data(), a.size());
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
            INFO(FSTR("arr={}", a));
            auto c = matrixChainMulti(a.data(), a.size() - 1);
            CHECK(c == b);
        }
    }

    SECTION("longest_common_subseq")
    {
        std::vector<std::tuple<std::string, std::string, std::string>> samples = {
            {"ABCBDAB", "BDCABA", "BCBA"},
        };
        for (auto& [a, b, c]: samples) {
            INFO(FSTR("a={}, b={}", a, b));
            auto r = longestCommonSubseq(a, b);
            CHECK(r == c);
        }
    }

    SECTION("longest_increasing_subseq")
    {
        std::vector<std::pair<std::vector<int>, std::vector<int>>> samples = {
            {{1}, {1}},
            {{2, 1}, {2}},
            {{1, 2, 3, 4, 5, 6, 7}, {1, 2, 3, 4, 5, 6, 7}},
            {{7, 6, 5, 4, 3, 2, 1}, {7}},
            {{3, 0, 5, 1, 2, 23, 43, 12, 31, 2, 3, 44, 5, 45, 46, 9},
             {0, 1, 2, 23, 43, 44, 45, 46}},
        };
        for (auto& [a, b]: samples) {
            INFO(FSTR("arr={}", a));
            auto c = longestIncreasingSubseq(a.data(), a.size());
            CHECK(b == c);
        }
    }

    SECTION("longest_palindrome_subseq")
    {
        std::vector<std::pair<std::string, std::string>> samples = {
            {"character", "carac"},
            {"xckiiqc", "ciic"},
            {"aibohphobia", "aibohphobia"},
            {"xrbascedceafrhd", "raecear"},
        };
        for (auto& [a, b]: samples) {
            INFO(FSTR("s={}", a));
            auto c = longestPalindromeSubseq(a);
            CHECK(b == c);
        }
    }
}

int main(int argc, char* argv[])
{
    Catch::Session session;
    auto& config = session.configData();
    config.shouldDebugBreak = true;
    int return_code = session.applyCommandLine(argc, argv);
    if (return_code != 0) {
        return return_code;
    }
    int num_failed = session.run();
    return num_failed;
}
