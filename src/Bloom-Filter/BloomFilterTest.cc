#include <iostream>

#include "BloomFilter.hh"

using namespace std;

int main() {
    BloomFilter<int, 10000> filter{};

    for (int i = 0; i < 10; i++)
        filter.Insert(i);

    for (int i = 0; i < 1000; i++)
        cout << i << " " << filter.Contains(i) << endl;

    return 0;
}