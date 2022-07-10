#ifndef BLOOM_H
#define BLOOM_H

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <array>
#include <math.h>

using namespace std;
template<class Key>
class BloomFilter{
public:
    BloomFilter(uint64_t n,double fp);
private:
    vector<bool> bits_;
};

#endif