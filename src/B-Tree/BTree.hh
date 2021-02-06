//
// Created by zhuo1ang on 1/18/21.
//

#ifndef DBDS_BTREE_HH
#define DBDS_BTREE_HH

#include <functional>
#include <optional>
#include <vector>

using namespace std;

template <typename K, typename V> class BTree {
private:
  static constexpr int min_tval = 2;
  static constexpr int default_tval = 32;
  static constexpr int max_tval = 256;

  class Node {
    vector<pair<K, V>> kvs_;
    vector<Node *> children_;
    bool leaf_;
  };

  Node *root_;
  int t_;

  void SplitNode(Node *, int, Node *);

  void InsertNonfull(Node *, K, V);

  void DeleteKey(Node *, K);

  pair<Node *, int> Search(Node *, K) const;

public:
  BTree(int t = default_tval);

  ~BTree();

  optional<reference_wrapper<V>> operator[](K) const;

  void Insert(K, V);

  void Remove(K);
};

template <typename K, typename V> BTree<K, V>::BTree(int t) {
  if (t < min_tval || t > max_tval)
    t_ = default_tval;
  else
    t_ = t;
  auto r = new BTree<K, V>::Node;
  r->leaf_ = true;
  root_ = r;
}

template <typename K, typename V>
void BTree<K, V>::SplitNode(Node *parent, int idx, Node *child) {
  auto new_node = new BTree<K, V>::Node;
  new_node->leaf_ = child->leaf_;
  new_node->kvs_ = vector(child->kvs_.begin() + t_ + 1, child->kvs_.end());
  if (not child->leaf_) {
    new_node->child_ =
        vector(child->children_.begin() + t_ + 1, child->children_.end());
  }
  parent->children_.insert(parent->children_.begin() + idx + 1, new_node);
  parent->kvs_.insert(parent->kvs_.children_begin() + idx, child->kvs_[idx]);
}

template <typename K, typename V>
void BTree<K, V>::InsertNonfull(Node *node, K key, V value) {
  int pos = node->kvs_.size();
  if (node->leaf_) {
    auto it = lower_bound(node->kvs_.begin(), node->kvs_.end(), key);
    node->kvs_.insert(it, key);
  } else {
    auto it = lower_bound(node->kvs_.begin(), node->kvs_.end(), key);
    int idx = it - node->kvs_.begin() + 1;
    if (node->children_[idx]->children_.size() == 2 * t_ - 1) {
      SplitNode(node, idx, node->children_[idx]);
      if (key > node->kvs_[idx].first) {
        idx++;
      }
    }
    InsertNonfull(node->children_[idx], key);
  }
}

template <typename K, typename V>
void BTree<K, V>::DeleteKey(Node *node, K Key) {
  // ......
}

template <typename K, typename V>
pair<typename BTree<K, V>::Node *, int>
BTree<K, V>::Search(BTree<K, V>::Node *node, K key) const {

  auto it = lower_bound(node->kvs_.begin(), node->kvs_.end(), key);

  if (it != node->kvs_.end() and key == (*it).first)
    return optional<pair<BTree<K, V>::Node *, int>>{
        make_pair(node, it - node->kvs_.begin())};

  if (node->leaf_)
    return make_pair(nullptr, 0);

  return Search(node->children_[it - node->kvs_.begin()], key);
}

template <typename K, typename V>
optional<reference_wrapper<V>> BTree<K, V>::operator[](K key) const {
  auto search_result = Search(root_, key);
  if (search_result.first != nullptr)
    return ref(search_result.first->kvs_[search_result.second]);
  else
    return nullopt;
}

template <typename K, typename V> void BTree<K, V>::Insert(K key, V value) {
  auto org_root = root_;
  if (org_root->kvs_.size() == 2 * t_ - 1) {
    auto new_root = new BTree<K, V>::Node;
    root_ = new_root;
    new_root->leaf_ = false;
    new_root->children_.push_front(org_root);
    SplitNode(new_root, 1, org_root);
    InsertNonfull(new_root, key, value);
  } else {
    InsertNonfull(root_, key, value);
  }
}

template <typename K, typename V> void BTree<K, V>::Remove(K key) {
  DeleteKey(root_, key);
}

#endif // DBDS_BTREE_HH
