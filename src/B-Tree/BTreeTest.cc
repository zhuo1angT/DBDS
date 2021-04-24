#include <iostream>
#include <string>

#include "BTree.hh"

using namespace std;

int main() {
  BTree<int, int> tree;
  for (int i = 0; i < 10; i++) {
    tree.Set(i, i * i);
  }
  for (int i = 0; i < 10; i++) {
    cout << tree.Get(i).value() << endl;
  }
  return 0;
}