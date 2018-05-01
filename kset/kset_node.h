#pragma once

#include <inttypes.h>
#include <tuple>
#include <memory>
#include <limits>
#include "errors.h"
#include "packed_ptr.h"
#include "gtest/gtest_prod.h"

#ifdef USE_AVX2
#include <immintrin.h>
#endif

namespace Kset {

class alignas(64) Node {

    FRIEND_TEST(NodeTest, basic_insertion);

  public:
    static constexpr unsigned capacity = 6;

  private:
    static constexpr int cache_line_size = 64;
    Node* children_{nullptr};
    PackedPtr parent_;
    int64_t vals_[capacity];

  public:
    Node* children() const {
        return children_;
    }

    uint16_t numValues() const  {
        //We are stealing 16 bits from the parent ptr to store the numValues
        return parent_.getData();
    }

    void incrementNumValues() {
        parent_.setData(parent_.getData() + 1);
    }

    bool isFull() const {
        return numValues() == capacity;
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
                for(int i = numValues(); i > idx; i--) {
                    vals_[i] = vals_[i-1];
                }
                vals_[idx] = val;
                incrementNumValues();
                found = true;
            }
        }

        return {idx, found};
    }

#ifndef USE_AVX2
    std::tuple<int,bool> find(int64_t val) const {
        int idx = 0;
        for(idx = 0; idx < numValues() && vals_[idx] < val; idx++);
        if(idx == numValues() || vals_[idx] != val) {
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

static_assert(sizeof(Node) == 64, "sizeof(Node) == 64");

}
