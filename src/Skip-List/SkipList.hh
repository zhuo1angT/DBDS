#ifndef DBDS_SKIPLIST_HH
#define DBDS_SKIPLIST_HH

#include <vector>
#include <optional>
#include <memory>
#include <random>
#include <algorithm>

template<typename K, typename V>
class SkipList {
private:

    static constexpr double P_MIN = 0.01;
    static constexpr double P_MAX = 0.80;
    static constexpr int MAX_LAYER_NUM = 16;

    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<double> dist;

    double p_;
    size_t levels_num_;
    size_t size_;


    class Node {
    public:
        enum NodeType {
            minus_inf, inf, normal
        };
        K key_;
        V value_;
        NodeType type_;
        Node *left_, *right_, *above_, *below_;

        Node() = default;

        explicit Node(NodeType type) {
            type_ = type;
        }

        Node(const K &key, const V &value, NodeType type) {
            key_ = key;
            value_ = value;
            type_ = type;
        }

        ~Node() = default;
    };

    std::vector<std::unique_ptr<Node>> all_nodes_;
    std::vector<Node *> heads_;

    Node *InsertNodeAtLeft(Node *, const K &, const V &);

    // return a single element vector when the container has the ideal key.
    // otherwise, return a vector with the pointers in each levels whose key
    // is greater than the given one.
    std::vector<Node *> Locate(const K &) const;

    int GetRandomLayerNum();

public:

    explicit SkipList(double p = 0.25);

    ~SkipList() = default;

    void Set(const K &, const V &);

    std::optional<std::reference_wrapper<V>> Get(const K &) const;

    std::optional<std::reference_wrapper<V>> operator[](const K &) const;

    [[nodiscard]] size_t Size() const;

};

template<typename K, typename V>
SkipList<K, V>::SkipList(double p) {
    mt = std::mt19937(rd());
    dist = std::uniform_real_distribution<double>(0.0, 1.0);

    if (p < P_MIN)
        p_ = P_MIN;
    else if (p > P_MAX)
        p_ = P_MAX;
    else
        p_ = p;
    levels_num_ = 1;
    size_ = 0;

    auto initial_minus_inf = new Node(Node::NodeType::minus_inf);
    auto initial_inf = new Node(Node::NodeType::inf);
    heads_.push_back(initial_minus_inf);

    all_nodes_.push_back(std::move(std::unique_ptr<Node>(initial_minus_inf)));
    all_nodes_.push_back(std::move(std::unique_ptr<Node>(initial_inf)));
}

template<typename K, typename V>
typename SkipList<K, V>::Node *SkipList<K, V>::InsertNodeAtLeft(Node *node, const K &key, const V &value) {
    auto new_node = new Node;
    new_node->key_ = key;
    new_node->value_ = value;
    new_node->left_ = node->left_;
    new_node->right = node;
    node->left_->right_ = new_node;
    node->left = new_node;

    all_nodes_.push_back(std::move(new_node));
}

template<typename K, typename V>
void SkipList<K, V>::Set(const K &key, const V &value) {
    auto locate_result = Locate(key);
    if (locate_result.size() == 1 && locate_result[0]->key_ == key) {
        auto node = locate_result[0];
        while (node != nullptr) {
            node->value_ = value;
            node = node->above_;
        }
        node = locate_result[0];
        while (node != nullptr) {
            node->value_ = value;
            node = node->below_;
        }
        return;
    } else {
        auto layer = GetRandomLayerNum();
        Node *prev_node = nullptr;
        Node *new_node = nullptr;
        for (int i = levels_num_ - layer; i < levels_num_; i++) {
            new_node = InsertNodeAtLeft(locate_result[i]);
            new_node->above_ = prev_node;
            if (prev_node != nullptr) // just for the first round of iteration
                prev_node->below_ = new_node;
            prev_node = new_node;
        }
        new_node->below_ = nullptr;
    }
}

template<typename K, typename V>
std::optional<std::reference_wrapper<V>> SkipList<K, V>::Get(const K &key) const {
    auto locate_result = Locate(key);
    if (locate_result.size() == 1 && locate_result[0]->key_ == key)
        return std::ref(locate_result[0]->value_);
    else
        return std::nullopt;
}

template<typename K, typename V>
std::optional<std::reference_wrapper<V>> SkipList<K, V>::operator[](const K &key) const {
    return Get(key);
}

template<typename K, typename V>
size_t SkipList<K, V>::Size() const {
    return size_;
}

template<typename K, typename V>
std::vector<typename SkipList<K, V>::Node *> SkipList<K, V>::Locate(const K &key) const {
    Node *cur_node = heads_[0];
    std::vector<Node *> ret;
    while (cur_node != nullptr) {
        auto next_node = cur_node->right_;
        if (key == cur_node->key) {
            return {cur_node};
        } else if (key < next_node->key_ || next_node->type_ == Node::NodeType::inf) {
            ret.push_back(next_node);
            cur_node = cur_node->below_;
        } else { // key >= next key
            cur_node = cur_node->right_;
        }
    }
    return ret;
}

template<typename K, typename V>
int SkipList<K, V>::GetRandomLayerNum() {
    int coin_flip_num = 1;
    while (dist(mt) < p_) {
        coin_flip_num++;
        if (coin_flip_num >= MAX_LAYER_NUM) break;
    }
    return coin_flip_num;
}


#endif  // DBDS_SKIPLIST_HH
