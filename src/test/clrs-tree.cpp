#include "utils/base.h"
#include <fmt/ranges.h>
#include <functional>
#include <stack>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

struct BinaryTree
{
    int key;
    BinaryTree* parent;
    BinaryTree* left;
    BinaryTree* right;
    bool black;  // red or black

    explicit BinaryTree(int key = 0, BinaryTree* parent = nullptr, BinaryTree* left = nullptr,
                        BinaryTree* right = nullptr, bool black = true)
        : key(key), parent(parent), left(left), right(right), black(black)
    {
    }
};

struct BinarySearchTree
{
    BinaryTree* root;
    std::set<BinaryTree*> pool;

    BinarySearchTree(): root(nullptr) {}

    BinarySearchTree(int* arr, int n): BinarySearchTree()
    {
        for (int i = 0; i < n; ++i) {
            insert(arr[i]);
        }
    }

    ~BinarySearchTree()
    {
        for (auto p: pool) delete p;
    }

    static void inorder_walk(BinaryTree* a, std::vector<int>& vec)
    {
        std::stack<BinaryTree*> sk;
        while (a != nullptr || !sk.empty()) {
            while (a != nullptr) {
                sk.push(a);
                a = a->left;
            }
            BinaryTree* b = sk.top();
            sk.pop();
            vec.push_back(b->key);
            a = b->right;
        }
    }

    void inorder_walk(std::vector<int>& vec) const { inorder_walk(root, vec); }

    static BinaryTree* get(BinaryTree* a, int i)
    {
        while (a != nullptr) {
            int j = a->key;
            if (j == i) {
                return a;
            } else if (i >= j) {
                a = a->right;
            } else {
                a = a->left;
            }
        }
        return nullptr;
    }

    BinaryTree* get(int i) const { return get(root, i); }

    static BinaryTree* max(BinaryTree* a)
    {
        if (a == nullptr) return a;
        while (a->right) {
            a = a->right;
        }
        return a;
    }

    BinaryTree* max() const { return max(root); }

    static BinaryTree* min(BinaryTree* a)
    {
        if (a == nullptr) return a;
        while (a->left) {
            a = a->left;
        }
        return a;
    }

    BinaryTree* min() const { return min(root); }

    static BinaryTree* successor(BinaryTree* tr)
    {
        if (!tr) return nullptr;
        auto a = min(tr->right);
        if (a) return a;
        auto p = tr->parent;
        auto c = tr;
        while (p != nullptr && p->right == c) {
            c = p;
            p = p->parent;
        }
        return p;
    }

    static BinaryTree* predecessor(BinaryTree* tr)
    {
        if (!tr) return nullptr;
        auto a = max(tr->left);
        if (a) return a;
        auto p = tr->parent;
        auto c = tr;
        while (p != nullptr && p->left == c) {
            c = p;
            p = p->parent;
        }
        return p;
    }

    static bool check_all(BinaryTree* a, int k, bool greater = true)
    {
        if (a == nullptr) {
            return true;
        }
        std::function<bool(int, int)> op = std::greater<int>();
        if (!greater) op = std::less_equal<int>();
        if (op(k, a->key)) return false;
        return check_all(a->left, k, greater) && check_all(a->right, k, greater);
    }

    static bool check(BinaryTree* a)
    {
        if (a == nullptr) return true;
        return check_all(a->left, a->key, false) && check_all(a->right, a->key, true) &&
               check(a->left) && check(a->right);
    }

    bool check() const { return check(this->root); }

    virtual BinaryTree* insert(int key)
    {
        BinaryTree* a = nullptr;
        BinaryTree* b = this->root;
        while (b != nullptr) {
            a = b;
            if (key >= b->key) {
                b = b->right;
            } else {
                b = b->left;
            }
        }
        auto n = new BinaryTree(key, a);
        pool.insert(n);
        if (a == nullptr) {
            root = n;
        } else if (key >= a->key) {
            a->right = n;
        } else {
            a->left = n;
        }
        return n;
    };

    void transplant(BinaryTree* a, BinaryTree* b)
    {
        auto p = a->parent;
        if (!p) {
            this->root = b;
        } else if (p->left == a) {
            p->left = b;
        } else if (p->right == a) {
            p->right = b;
        }
        if (b) b->parent = p;
        a->parent = nullptr;
    }

    virtual void erase(BinaryTree* node)
    {
        if (!node) return;
        if (!node->left) {
            transplant(node, node->right);
        } else if (!node->right) {
            transplant(node, node->left);
        } else {
            auto s = successor(node);
            if (node->right != s) {
                transplant(s, s->right);
                s->right = node->right;
                node->right->parent = s;
            }
            transplant(node, s);
            s->left = node->left;
            node->left->parent = s;
        }
    }
};

struct RedBlackTree
{
    BinaryTree* root;
    std::set<BinaryTree*> pool;
    BinaryTree* nil;

    RedBlackTree(int* arr, int n): RedBlackTree()
    {
        for (int i = 0; i < n; ++i) {
            insert(arr[i]);
        }
    }

    RedBlackTree()
    {
        nil = new BinaryTree;
        pool.insert(nil);
        nil->black = true;
        root = nil;
    }

    ~RedBlackTree() = default;

    void inorder_walk(BinaryTree* a, std::vector<int>& vec) const
    {
        std::stack<BinaryTree*> sk;
        while (a != nil || !sk.empty()) {
            while (a != nil) {
                sk.push(a);
                a = a->left;
            }
            BinaryTree* b = sk.top();
            sk.pop();
            vec.push_back(b->key);
            a = b->right;
        }
    }

    void inorder_walk(std::vector<int>& vec) const { inorder_walk(root, vec); }

    bool check_all(BinaryTree* a, int k, bool greater = true)
    {
        if (a == nil) {
            return true;
        }
        std::function<bool(int, int)> op = std::greater<int>();
        if (!greater) op = std::less<int>();
        if (op(k, a->key)) return false;
        return check_all(a->left, k, greater) && check_all(a->right, k, greater);
    }

    bool check(BinaryTree* a)
    {
        if (a == nil) return true;
        return check_all(a->left, a->key, false) && check_all(a->right, a->key, true) &&
               check(a->left) && check(a->right);
    }

    bool check() { return check(this->root); }

    void left_rotate(BinaryTree* x)
    {
        assert(x->right != nil && root->parent == nil);
        auto y = x->right;
        x->right = y->left;
        if (y->left != nil) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        } else if (x->parent->left == x) {
            x->parent->left = y;
        } else if (x->parent->right == x) {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void right_rotate(BinaryTree* y)
    {
        assert(y->left != nil && root->parent == nil);
        auto x = y->left;
        y->left = x->right;
        if (x->right != nil) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == nil) {
            root = x;
        } else if (y->parent->left == y) {
            y->parent->left = x;
        } else if (y->parent->right == y) {
            y->parent->right = x;
        }
        x->right = y;
        y->parent = x;
    }

    void insert_fixup(BinaryTree* z)
    {
        while (!z->parent->black) {
            if (z->parent == z->parent->parent->left) {
                auto y = z->parent->parent->right;
                if (!y->black) {
                    z->parent->black = true;
                    y->black = true;
                    z->parent->parent->black = false;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->black = true;
                    z->parent->parent->black = false;
                    right_rotate(z->parent->parent);
                }
            } else {
                auto y = z->parent->parent->left;
                if (!y->black) {
                    z->parent->black = true;
                    y->black = true;
                    z->parent->parent->black = false;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->black = true;
                    z->parent->parent->black = false;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root->black = true;
    }

    BinaryTree* insert(int key)
    {
        BinaryTree* a = nil;
        BinaryTree* b = root;
        while (b != nil) {
            a = b;
            if (key >= b->key) {
                b = b->right;
            } else {
                b = b->left;
            }
        }
        BinaryTree* n = new BinaryTree(key, a, nil, nil, false);
        pool.insert(n);
        if (a == nil) {
            root = n;
        } else if (key >= a->key) {
            a->right = n;
        } else {
            a->left = n;
        }
        insert_fixup(n);
        return n;
    }

    void erase(BinaryTree* node) {}
};

TEST_CASE("binary_search_tree")
{
    std::vector<std::pair<std::vector<int>, std::vector<int>>> samples = {
        {{}, {}},
        {{6}, {6}},
        {{5, 4, 2, 1, 2}, {1, 2, 2, 4, 5}},
        {{1, 2, 3, 4, 5, 6, 7, 8}, {1, 2, 3, 4, 5, 6, 7, 8}},
        {{10000, 1000, 100, 10, 1}, {1, 10, 100, 1000, 10000}},
        {{16, 4, 2, 1, 8}, {1, 2, 4, 8, 16}},
        {{1024, 64, 256, 6553, 8}, {8, 64, 256, 1024, 6553}},
        {{5, 1, -1, 1, 5}, {-1, 1, 1, 5, 5}},
        {{1, 2, 1, 2, 1}, {1, 1, 1, 2, 2}},
        {{1024, 64, -256, 6553, 8, -1, 92, 3, 3, -78, 23},
         {-256, -78, -1, 3, 3, 8, 23, 64, 92, 1024, 6553}},
    };

    SECTION("red_black")
    {
        for (auto& [a, b]: samples) {
            INFO(fmt::format("arr={}", a));
            RedBlackTree rb(a.data(), a.size());
            REQUIRE(rb.check());
            std::vector<int> c;
            rb.inorder_walk(c);
            REQUIRE(c == b);
        }
    }

    SECTION("plain")
    {
        for (auto& [a, b]: samples) {
            INFO(fmt::format("arr={}", a));
            BinarySearchTree bt(a.data(), a.size());
            REQUIRE(bt.check());
            std::vector<int> c;
            bt.inorder_walk(c);
            REQUIRE(c == b);
            if (a.size() == 0) continue;
            auto node = bt.min();
            REQUIRE(node->key == b[0]);
            for (int i = 1; i < b.size(); ++i) {
                node = BinarySearchTree::successor(node);
                REQUIRE(node->key == b[i]);
            }
            REQUIRE(BinarySearchTree::successor(node) == nullptr);
            node = bt.max();
            REQUIRE(node->key == b.back());
            for (int i = b.size() - 2; i >= 0; --i) {
                node = BinarySearchTree::predecessor(node);
                REQUIRE(node->key == b[i]);
            }
            REQUIRE(BinarySearchTree::predecessor(node) == nullptr);
        }
        std::vector<int> vec = {1024, 64, -256, 6553, 8, -1, 92, 3, 3, -78, 23};
        BinarySearchTree bt(vec.data(), vec.size());
        REQUIRE(bt.check());
        REQUIRE(bt.get(92)->key == 92);
        REQUIRE(bt.get(2) == nullptr);
        REQUIRE(bt.get(93) == nullptr);
        REQUIRE(bt.get(-78)->key == -78);
        REQUIRE(bt.get(64)->key == 64);
        REQUIRE(bt.max()->key == 6553);
        REQUIRE(bt.min()->key == -256);
        auto nb = bt.insert(93);
        REQUIRE(bt.check());
        REQUIRE(BinarySearchTree::successor(nb)->key == 1024);
        REQUIRE(BinarySearchTree::predecessor(nb)->key == 92);
        vec.push_back(93);
        std::sort(vec.begin(), vec.end());
        for (int i = 0; i < vec.size(); ++i) {
            bt.erase(bt.get(vec[i]));
            std::vector<int> aft;
            bt.inorder_walk(aft);
            std::vector<int> ans(vec.begin() + i + 1, vec.end());
            REQUIRE(aft == ans);
        }
    }
}

int main(int argc, char* argv[])
{
    Catch::Session session;
    auto& config = session.configData();
    config.shouldDebugBreak = true;
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) return returnCode;
    int numFailed = session.run();
    return numFailed;
}