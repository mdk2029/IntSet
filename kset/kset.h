#pragma once

#include <inttypes.h>
#include <cstdlib>
#include <cassert>
#include <tuple>
#include <memory>
#include <cstring>
#include <iostream>
#include <sstream>
#include <limits>

#ifdef USE_AVX2
#include <immintrin.h>
#endif

#ifndef NDEBUG

#define ASSERT_IMPLIES(X,Y) \
    if(!(X) || (Y)) {       \
    }  else {               \
      throw std::logic_error("assert_implies failed :[ " #X " ] [ " #Y " ]"); \
    }

#define ASSERT(X) \
    if(!(X)) { \
        throw std::logic_error("assert failed: [" #X "]"); \
    }

#else

#define ASSERT_IMPLIES(X,Y)
#define ASSERT(X)

#endif

namespace Kset {

struct alignas(64) Node {
    static constexpr unsigned capacity = 6;
    static constexpr int cache_line_size = 64;
    Node* children_{nullptr};
    uint32_t pad1_ {0};
    uint16_t pad2_ {0};
    uint16_t numValues_ {0};
    int64_t vals_[capacity];

    bool isFull() const {
        return numValues_ == capacity;
    }

    void expand() {
        ASSERT(!children_);
        children_ = new Node[capacity+1]{};
    }

    std::tuple<int,bool> insert(int64_t val) {
        bool found{false};
        int idx{-1};

        std::tie(idx,found) = find(val);
        if(!found) {
            if(!isFull()) {
                for(int i = numValues_; i > idx; i--) {
                    vals_[i] = vals_[i-1];
                }
                vals_[idx] = val;
                ++numValues_;
                found = true;
            }
        }

        return {idx, found};
    }

#ifndef USE_AVX2
    std::tuple<int,bool> find(int64_t val) const {
        int idx = 0;
        for(idx = 0; idx < numValues_ && vals_[idx] < val; idx++);
        if(idx == numValues_ || vals_[idx] != val) {
            return {idx, false};
        } else {
            return {idx,true};
        }
    }
#else
    //Use AVX2 instructions for optimized find
    std::tuple<int,bool> find(int64_t val) const;
    Node();
#endif

    void* operator new[](size_t size);
    void* operator new(size_t size);

    void operator delete(void* p);
    void operator delete[](void* p);
    ~Node();
};

std::tuple<Node*, int, bool> find(Node* node, int64_t val);
std::tuple<Node*, int, bool> insert(Node* node, int64_t val);

#ifdef USE_AVX2

inline
Node::Node() {
    for(unsigned i = 0; i < capacity; i++) {
        vals_[i] = std::numeric_limits<int64_t>::max();
    }
}

inline
std::tuple<int,bool> Node::find(int64_t val) const {

    bool found{false};
    int idx{0};

    constexpr int64_t maxint64 = std::numeric_limits<int64_t>::max();
    __m256i valsp = _mm256_load_si256(reinterpret_cast<const __m256i*>(this));
    __m256i targetp = {maxint64, maxint64,val,val};
    __m256i maskeqp = _mm256_cmpgt_epi64(valsp,targetp);
    int mask = _mm256_movemask_epi8(maskeqp);

    if(mask != 0) {
        //we have found the value/branching point in the first 32 bytes of the cache line
        unsigned trailingZeroes = __builtin_ctz(static_cast<uint32_t>(mask));
        unsigned firstQuad = trailingZeroes / 8;
        ASSERT(firstQuad > 1);
        idx = firstQuad - 2;
        found = idx > 0 ? vals_[idx-1] == val : false;
        idx = found ? idx-1 : idx;
    } else {
        //we need to look in the trailing 32 bytes of the cache line.
        valsp = _mm256_load_si256(reinterpret_cast<const __m256i*>(&vals_[2]));
        targetp = _mm256_set1_epi64x(val);
        maskeqp = _mm256_cmpgt_epi64(valsp,targetp);
        int mask = _mm256_movemask_epi8(maskeqp);

        unsigned trailingZeroes = mask ? __builtin_ctz(static_cast<uint32_t>(mask))
                                       : 32;
        unsigned firstQuad = (trailingZeroes / 8);
        idx = firstQuad + 2;
        found = vals_[idx-1] == val ? true : false;
        idx = found ? idx -1 : idx;
    }
    return {idx,found};
}

#endif

}

