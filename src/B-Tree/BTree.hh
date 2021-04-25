#ifndef DBDS_BTREE_HH
#define DBDS_BTREE_HH

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>
#include <memory>

template<typename K, typename V>
class BTree {
private:
    static constexpr int MIN_TVAL = 2;
    static constexpr int DEFAULT_TVAL = 32;
    static constexpr int MAX_TVAL = 64;
    static constexpr auto COMP = [](const std::pair<K, V> &p, const K &k) {
        return p.first < k;
    };

    class Node {
    public:
        std::vector<std::pair<K, V>> kvs_;
        std::vector<std::shared_ptr<Node>>
                children_;
        bool leaf_;
    };

    std::shared_ptr<Node> root_;
    uint8_t t_;
    uint32_t size_;

    void SplitChild(std::shared_ptr<Node>, int);

    void InsertNonfull(std::shared_ptr<Node>, const K &, const V &);

    void DeleteKey(std::shared_ptr<Node>, const K &);

    std::optional<std::pair<std::shared_ptr<Node>, uint32_t>> Search(std::shared_ptr<Node>, const K &) const;


public:
    explicit BTree(int t = DEFAULT_TVAL);

    ~BTree() = default;

    void Set(const K &, const V &);

    std::optional<std::reference_wrapper<V>> Get(const K &) const;

    std::optional<std::reference_wrapper<V>> operator[](const K &) const;

    void Remove(const K &);

    [[nodiscard]] uint32_t Size() const;

    [[nodiscard]] bool Empty() const;
};

template<typename K, typename V>
BTree<K, V>::BTree(int t) {
    if (t < MIN_TVAL)
        t_ = MIN_TVAL;
    else if (t > MAX_TVAL)
        t_ = MAX_TVAL;
    else
        t_ = t;

    root_ = std::make_shared<BTree<K, V>::Node>();
    root_->leaf_ = true;
    size_ = 0;
}


template<typename K, typename V>
void BTree<K, V>::SplitChild(std::shared_ptr<Node> parent, int idx) {
    auto new_node = std::make_shared<BTree<K, V>::Node>();
    auto child = parent->children_[idx];
    new_node->leaf_ = child->leaf_;
    new_node->kvs_ = std::vector(child->kvs_.begin() + t_, child->kvs_.end());
    if (not child->leaf_)
        new_node->children_ =
                std::vector(child->children_.begin() + t_, child->children_.end());

    parent->children_.insert(parent->children_.begin() + idx + 1, new_node);
    parent->kvs_.insert(parent->kvs_.begin() + idx, child->kvs_[t_ - 1]);
    parent->children_[idx]->kvs_.resize(t_ - 1);
    parent->children_[idx]->children_.resize(t_);
}

template<typename K, typename V>
void BTree<K, V>::InsertNonfull(std::shared_ptr<Node> node, const K &key, const V &value) {
    auto iter = std::lower_bound(node->kvs_.begin(), node->kvs_.end(), key, COMP);
    if (iter != node->kvs_.end() and iter->first == key) {
        iter->second = value;
        return;
    }
    if (node->leaf_) {
        node->kvs_.insert(iter, std::make_pair(key, value));
        size_ += 1;
        return;
    }

    int idx = iter - node->kvs_.begin();
    if (node->children_[idx]->kvs_.size() == 2 * t_ - 1) {
        SplitChild(node, idx);
        if (key > node->kvs_[idx].first)
            idx++;
    }
    InsertNonfull(node->children_[idx], key, value);
}

template<typename K, typename V>
void BTree<K, V>::DeleteKey(std::shared_ptr<Node> node, const K &Key) {
    // ......
}

template<typename K, typename V>
std::optional<std::pair<std::shared_ptr<typename BTree<K, V>::Node>, uint32_t>>
BTree<K, V>::Search(std::shared_ptr<BTree<K, V>::Node> node, const K &key) const {
    auto iter = std::lower_bound(node->kvs_.begin(), node->kvs_.end(), key, COMP);
    if (iter != node->kvs_.end() and key == iter->first)
        return std::make_pair(node, iter - node->kvs_.begin());
    if (node->leaf_)
        return std::nullopt;
    else
        return Search(node->children_[iter - node->kvs_.begin()], key);
}

template<typename K, typename V>
std::optional<std::reference_wrapper<V>> BTree<K, V>::Get(const K &key) const {
    auto search_result = Search(root_, key);
    if (search_result.has_value())
        return std::ref(
                search_result.value().first->kvs_[search_result.value().second].second);
    else
        return std::nullopt;
}

template<typename K, typename V>
std::optional<std::reference_wrapper<V>> BTree<K, V>::operator[](
        const K &key) const {
    return Get(key);
}

template<typename K, typename V>
void BTree<K, V>::Set(const K &key, const V &value) {
    auto org_root = root_;
    if (org_root->kvs_.size() == 2 * t_ - 1) {
        auto new_root = std::make_shared<BTree<K, V>::Node>();
        root_ = new_root;
        new_root->leaf_ = false;
        new_root->children_.push_back(org_root);
        SplitChild(new_root, 0);
    }
    InsertNonfull(root_, key, value);
}

template<typename K, typename V>
void BTree<K, V>::Remove(const K &key) {
    DeleteKey(root_, key);
}

template<typename K, typename V>
uint32_t BTree<K, V>::Size() const {
    return size_;
}

template<typename K, typename V>
bool BTree<K, V>::Empty() const {
    return size_ == 0;
}

#endif  // DBDS_BTREE_HH
