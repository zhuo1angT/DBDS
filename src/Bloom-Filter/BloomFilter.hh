#ifndef DBDS_BLOOMFILTER_HH
#define DBDS_BLOOMFILTER_HH

#include <vector>
#include <functional>
#include <bitset>
#include <iostream>

template<typename T, size_t S>
class BloomFilter {
private:

    std::vector<std::function<size_t(T)>> hash_functions_;

    std::bitset<S> bitset_;

public:

    BloomFilter();

    BloomFilter(std::initializer_list<std::function<size_t(T)>>);

    void Insert(const T &);

    bool Contains(const T &) const;
};

template<typename T, size_t S>
bool BloomFilter<T, S>::Contains(const T &value) const {
    for (auto hash : hash_functions_)
        if (not bitset_[hash(value) % S])
            return false;
    return true;
}

template<typename T, size_t S>
BloomFilter<T, S>::BloomFilter() {
    hash_functions_.push_back(std::hash<T>());
}

template<typename T, size_t S>
BloomFilter<T, S>::BloomFilter(std::initializer_list<std::function<size_t(T)>> list) {
    for (auto hash : list)
        hash_functions_.push_back(hash);
}

template<typename T, size_t S>
void BloomFilter<T, S>::Insert(const T &value) {
    for (auto hash : hash_functions_)
        bitset_.set(hash(value) % S);
}


#endif //DBDS_BLOOMFILTER_HH
