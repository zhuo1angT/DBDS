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

    SkipList<int, int> skiplist(32);
    map<int, int> stdmap;

    double p = 0.3;

    for (int i = 0; i < N; i++) {
        if ((static_cast<double>(rand()) / RAND_MAX) > p) {
            int r1 = rand(), r2 = rand();
            skiplist.Set(r1, r2);
            stdmap[r1] = r2;
            // cout << "Insert " << r1 << " " << r2 << endl;
        } else if (!stdmap.empty() and !skiplist.Empty()) {
            auto iter = stdmap.lower_bound(rand());
            if (iter == stdmap.end()) iter = stdmap.begin();
            // cout << "Check " << iter->first << endl;
            if (stdmap[iter->first] != skiplist.Get(iter->first).value()) {
                // cout << "Value not equal" << endl;
                goto FAIL;
            }
            stdmap.erase(iter->first);
            skiplist.Remove(iter->first);
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