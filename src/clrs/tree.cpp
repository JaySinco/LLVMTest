#include "utils/base.h"
#include <fmt/ranges.h>
#include <stack>
#include <catch2/catch.hpp>

struct BinaryTree
{
    int key;
    BinaryTree* parent;
    BinaryTree* left;
    BinaryTree* right;

    BinaryTree(int key = 0, BinaryTree* parent = nullptr)
    {
        this->left = nullptr;
        this->right = nullptr;
        this->parent = parent;
        this->key = key;
    }

    ~BinaryTree()
    {
        if (left != nullptr) delete left;
        if (right != nullptr) delete right;
    }
};

struct BinarySearchTree
{
    BinaryTree* root;
    bool shouldDelete;

    static BinaryTree* build_up(int* arr, int n, BinaryTree* parent = nullptr)
    {
        if (n <= 0) {
            return nullptr;
        }
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
        auto tr = new BinaryTree;
        tr->key = arr[p];
        tr->parent = parent;
        tr->left = build_up(arr, p, tr);
        tr->right = build_up(arr + p + 1, n - p - 1, tr);
        return tr;
    }

    BinarySearchTree(int* arr, int n): root(build_up(arr, n)), shouldDelete(true) {}

    BinarySearchTree(BinaryTree* root, bool shouldDelete = false)
        : root(root), shouldDelete(shouldDelete)
    {
    }

    ~BinarySearchTree()
    {
        if (shouldDelete && root) delete root;
    }

    void inorder_walk(std::vector<int>& vec)
    {
        std::stack<BinaryTree*> sk;
        BinaryTree* a = root;
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

    BinaryTree* get(int i)
    {
        BinaryTree* a = root;
        while (a != nullptr) {
            int j = a->key;
            if (j == i) {
                return a;
            } else if (i > j) {
                a = a->right;
            } else {
                a = a->left;
            }
        }
        return nullptr;
    }

    BinaryTree* max()
    {
        BinaryTree* a = root;
        if (a == nullptr) return a;
        while (a->right) {
            a = a->right;
        }
        return a;
    }

    BinaryTree* min()
    {
        BinaryTree* a = root;
        if (a == nullptr) return a;
        while (a->left) {
            a = a->left;
        }
        return a;
    }

    static BinaryTree* successor(BinaryTree* tr)
    {
        if (!tr) return nullptr;
        auto a = BinarySearchTree(tr->right).min();
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
        auto a = BinarySearchTree(tr->left).max();
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
        if (!greater) op = std::less<int>();
        if (op(k, a->key)) return false;
        return check_all(a->left, k, greater) && check_all(a->right, k, greater);
    }

    static bool check(BinaryTree* a)
    {
        if (a == nullptr) return true;
        return check_all(a->left, a->key, false) && check_all(a->right, a->key, true) &&
               check(a->left) && check(a->right);
    }

    bool check() { return check(this->root); }

    BinaryTree* insert(int key)
    {
        BinaryTree* a = nullptr;
        BinaryTree* b = this->root;
        while (b != nullptr) {
            a = b;
            if (key > b->key) {
                b = b->right;
            } else {
                b = b->left;
            }
        }
        auto n = new BinaryTree(key, a);
        if (key > a->key) {
            a->right = n;
        } else {
            a->left = n;
        }
        return n;
    };

    void detach(BinaryTree* a, BinaryTree* b)
    {
        if (a == b) {
        }
    }

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
        delete a;
    }

    void erase(BinaryTree* node)
    {
        if (!node->left) {
            transplant(node, node->right);
        }
    }
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
    };
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
}
