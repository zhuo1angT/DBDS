#include <iostream>
#include <map>
#include <set>
#include <cstdlib>
#include <ctime>

#include "SkipList.hh"

using namespace std;

int main() {
    const int N = 100000;
    srand(time(nullptr)); // use current time as seed for random generator

    SkipList<int, int> skiplist;

    /*
    Insert 302767422 1059525948
    Insert 132271370 2138670618
    Check 132271370
    Insert 871459743 432698083
    Check 302767422
    Check 871459743
    */

    map<int, int> stdmap;

    double p = 0.3;

    for (int i = 0; i < N; i++) {
        if ((static_cast<double>(rand()) / RAND_MAX) > p) {
            int r1 = rand(), r2 = rand();
            skiplist.Set(r1, r2);
            stdmap[r1] = r2;
            // cout << "Insert " << r1 << " " << r2 << endl;
        } else if (!stdmap.empty() and !skiplist.Empty()) {
            if ((static_cast<double>(rand()) / RAND_MAX) > p) {
                auto rnd = rand();
                if ((stdmap.count(rnd) == 0 && !skiplist.Get(rnd).has_value()) ||
                    (stdmap.count(rnd) == 1 && skiplist.Get(rnd).has_value() &&
                     skiplist.Get(rnd).value() == stdmap[rnd])) { ;
                } else {
                    goto FAIL;
                }
            } else {
                auto iter = stdmap.lower_bound(rand());
                if (iter == stdmap.end()) iter = stdmap.begin();
                // cout << "Check " << iter->first << endl;
                if (stdmap[iter->first] != skiplist.Get(iter->first).value()) {
                    // cout << "Value not equal" << endl;
                    goto FAIL;
                }
                stdmap.erase(iter->first);
                skiplist.Remove(iter->first);
            }
        } else if (stdmap.size() != skiplist.Size()) {
            // cout << "Size not equal" << endl;
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