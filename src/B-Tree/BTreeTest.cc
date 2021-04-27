#include <iostream>
#include <map>
#include <set>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include "BTree.hh"

using namespace std;

int main() {
    const int N = 200000;
    srand(time(nullptr)); // use current time as seed for random generator

    BTree<int, int> btree(32);
    map<int, int> stdmap;

    double p = 0.5;

    for (int i = 0; i < N; i++) {
        if ((static_cast<double>(rand()) / RAND_MAX) > p) {
            int r1 = rand(), r2 = rand();
            btree.Set(r1, r2);
            stdmap[r1] = r2;
            // cout << "Insert " << r1 << " " << r2 << endl;
        } else if (!stdmap.empty() and !btree.Empty()) {
            if ((static_cast<double>(rand()) / RAND_MAX) > p) {
                auto rnd = rand();
                if ((stdmap.count(rnd) == 0 && !btree.Get(rnd).has_value()) ||
                    (stdmap.count(rnd) == 1 && btree.Get(rnd).has_value() &&
                     btree.Get(rnd).value() == stdmap[rnd])) { ;
                } else {
                    goto FAIL;
                }
            } else {
                auto iter = stdmap.lower_bound(rand());
                if (iter == stdmap.end()) iter = stdmap.begin();
                // cout << "Check " << iter->first << endl;
                if (stdmap[iter->first] != btree.Get(iter->first).value()) {
                    goto FAIL;
                }
            }
        } else if (stdmap.size() != btree.Size()) {
            goto FAIL;
        }
    }

    PASS:
    cout << "Test Passed " << endl;
    return 0;

    FAIL:
    cout << "Test Failed." << endl;
    return 0;
}