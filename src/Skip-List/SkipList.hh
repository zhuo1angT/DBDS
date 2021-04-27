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
    ssize_t levels_num_;
    ssize_t size_;


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

    void GenNewLevel();

    // Note that this method does *not* take care of the above_ & the below_ pointers
    Node *InsertNodeAtLeft(Node *, const K &, const V &);

    // Return a single element vector when the container has the ideal key.
    // otherwise, return a vector with the pointers in each levels whose key
    // is greater than the given one.
    std::vector<Node *> Locate(const K &) const;

    // Generate a randomized level number, for an insertion
    int GetRandomLevelNum();

public:

    explicit SkipList(double p = 0.25);

    ~SkipList() = default;

    void Set(const K &, const V &);

    std::optional<std::reference_wrapper<V>> Get(const K &) const;

    std::optional<std::reference_wrapper<V>> operator[](const K &) const;

    void Remove(const K &);

    [[nodiscard]] ssize_t Size() const;

    [[nodiscard]] bool Empty() const;
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
    initial_minus_inf->right_ = initial_inf;
    initial_inf->left_ = initial_minus_inf;
    heads_.push_back(initial_minus_inf);

    all_nodes_.push_back(std::move(std::unique_ptr<Node>(initial_minus_inf)));
    all_nodes_.push_back(std::move(std::unique_ptr<Node>(initial_inf)));
}

template<typename K, typename V>
typename SkipList<K, V>::Node *SkipList<K, V>::InsertNodeAtLeft(Node *node, const K &key, const V &value) {
    auto new_node = new Node(Node::NodeType::normal);
    new_node->key_ = key;
    new_node->value_ = value;
    new_node->left_ = node->left_;
    new_node->right_ = node;
    node->left_->right_ = new_node;
    node->left_ = new_node;

    all_nodes_.push_back(std::move(std::unique_ptr<Node>(new_node)));
    return new_node;
}

template<typename K, typename V>
void SkipList<K, V>::Set(const K &key, const V &value) {
    auto locate_result = Locate(key);
    if (locate_result.size() == 1 && locate_result[0]->type_ == Node::NodeType::normal &&
        locate_result[0]->key_ == key) {
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
    }
    auto cur_levels = GetRandomLevelNum();
    auto org_levels_num = levels_num_;

    while (levels_num_ < cur_levels)
        GenNewLevel();

    for (int i = levels_num_ - org_levels_num - 1; i >= 0; i--)
        locate_result.insert(locate_result.begin(), heads_[i]->right_);

    Node *prev_node = nullptr;
    Node *new_node = nullptr;
    for (int i = levels_num_ - 1; i >= levels_num_ - cur_levels; i--) {
        new_node = InsertNodeAtLeft(locate_result[i], key, value);
        new_node->below_ = prev_node;
        if (prev_node != nullptr) // just for the first round of iteration
            prev_node->above_ = new_node;
        prev_node = new_node;
    }
    new_node->above_ = nullptr;
    size_ += 1;
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
ssize_t SkipList<K, V>::Size() const {
    return size_;
}

template<typename K, typename V>
std::vector<typename SkipList<K, V>::Node *> SkipList<K, V>::Locate(const K &key) const {
    Node *cur_node = heads_[0];
    std::vector<Node *> ret;
    while (cur_node != nullptr) {
        auto next_node = cur_node->right_;
        if (key == cur_node->key_) {
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
int SkipList<K, V>::GetRandomLevelNum() {
    int coin_flip_num = 1;
    while (dist(mt) < p_) {
        coin_flip_num++;
        if (coin_flip_num >= MAX_LAYER_NUM) break;
    }
    return coin_flip_num;
}

template<typename K, typename V>
void SkipList<K, V>::GenNewLevel() {
    auto initial_minus_inf = new Node(Node::NodeType::minus_inf);
    auto initial_inf = new Node(Node::NodeType::inf);
    initial_minus_inf->right_ = initial_inf;
    initial_inf->left_ = initial_minus_inf;
    initial_minus_inf->below_ = heads_[0];
    initial_inf->below_ = heads_[0]->right_;
    heads_.insert(heads_.begin(), initial_minus_inf);
    levels_num_ += 1;

    all_nodes_.push_back(std::move(std::unique_ptr<Node>(initial_minus_inf)));
    all_nodes_.push_back(std::move(std::unique_ptr<Node>(initial_inf)));
}

template<typename K, typename V>
bool SkipList<K, V>::Empty() const {
    return size_ == 0;
}

template<typename K, typename V>
void SkipList<K, V>::Remove(const K &key) {
    auto locate_result = Locate(key);
    if (not(locate_result.size() == 1 && locate_result[0]->type_ == Node::NodeType::normal &&
            locate_result[0]->key_ == key)) {
        return;
    }
    auto node = locate_result[0];
    while (node != nullptr) {
        node->left_->right_ = node->right_;
        node->right_->left_ = node->left_;
        node = node->above_;
    }
    node = locate_result[0];
    while (node != nullptr) {
        node->left_->right_ = node->right_;
        node->right_->left_ = node->left_;
        node = node->below_;
    }
}


#endif  // DBDS_SKIPLIST_HH
