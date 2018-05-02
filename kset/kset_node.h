#pragma once

#include <inttypes.h>
#include <tuple>
#include <memory>
#include <limits>
#include "errors.h"
#include "packed_ptr.h"

#ifdef USE_AVX2
#include <immintrin.h>
#endif

/**
 * \defgroup Kset Integer Set designed to minimize memory accesses a.k.a simplified BTree
 *
 * This package is designed specifically for one thing - To maintain an ordered
 * collection of 64 bit ints. 64 bit ints occur quite frequently in many
 * programs (say object addresses, IDs etc). The default C++ mechanism to
 * maintain an ordered collection - a std::set - can be quite inefficient
 * because though it has good theoretical guarantees (namely those of Red Black trees)
 * , but, in practice, optimization often begins and ends with optimizing memory
 * accesses.
 *
 * A Kset Node will be exactly 64 bytes long and will hold 6 64 bit ints. Since memory
 * accesses usually work on a granularity of 64 byte cache lines, so, we try to
 * use all the fetched memory instead of storing just one 64 bit int in a node as is
 * done in a std::set<int64_t>.
 *
 * So now we have a node with 6 values. We now need 7 "pointers" to descend to the
 * appropriate child node from this node when we are traversing the tree. Instead of
 * actually storing 7 pointers, we will layout all 7 child nodes contiguously and
 * thus need to store only one pointer in our node.
 *
 * The layout is described next. We will further control memory allocation so that the 64 bytes
 * line up along a cache boundary
 * x-----8----x-----8----x-----8----x----8-----x----8-----x----8-----x----8-----x----8-----x
 * | childptr |parentptr |   val0   |   val1   |    val2  |   val3   |   val4   |   val5   |
 * x----------x----------x----------x----------x----------x----------x----------x----------x
 *
 * When inserting values in a node, we will ensure that we maintain a sorted order. So val0 < val1 < .... val4 < val5
 * Since we are dealing with data that is gurarnteed to be in one cache line (which has already been fetched), so
 * there is minimal overhead in maintaining the sorted ordering locally within a node.
 *
 * childptr will be pointing to a memory location where we will layout 7 such nodes (i.e. 7x64 bytes).
 * i.e. we have
 *
 * childptr--->|Node0|Node1|Node2|Node3|Node4|Node5|Node6|
 *
 * Thus, suppose we are looking for a value v such that val1 < v < val2. Then, we know that
 * this value must be in the tree rooted at the node (childptr + 3)
 *
 * We now describe parentptr next.
 *
 * We need parentptr because we are going to support the successor() operation. Logically, the parentptr
 * is simple - for all the Node0,Node1 etc described above, the parentptr will be the same since they
 * all have the same parent.
 *
 * However, there is one final twist here. Note that in general, we need to know how many values are currently
 * present in a node. (because, we will expand a node only after we have inserted 6 values into it)
 * But we are already out of space since we have taken up 64 bytes so far. So, we will resort to a x86_64 specific
 * hack. We know that the top 16 bits of a pointer are not used. So we will steal the top 16 bits of parentptr
 * and use them to keep track of numValues
 *
 * The main advantage of the above design is to better utilize all memory that we touch (i.e. that gets fetched into cache)
 * An added (relatively small) advantage is that we can use AVX2/AVX512 instructions to find the branching point when
 * searching for a value. We provide a CMake option to enable using AVX2. We can compare 4 64 bit ints at one shot and thus
 * ideally need only 2 comparisons (to cover 64 bytes) to find the branching point (i.e. which child node to descend to)
 *
 **/

namespace Kset {

//todo Use strong typedefs for these

///The location of a value is given by a pointer to a node and the index within that node
using NodeIdx_t = uint16_t;
static constexpr NodeIdx_t invalid_idx = std::numeric_limits<NodeIdx_t>::max();

/// We will store int64_t values. This class is purposefully desigined for one thing only
/// and hence there is no attempt to generalize things. This is deliberate and reflects
/// my opinion that a lot of C++ code is hard to read because of unnecessary genericity
using val_t = int64_t;

/**
 * @brief The Node class
 *
 * This is the fundamental Node struct in our tree. Please see package description for design details
 *
 * Note that as of C++14, alignas is not required to honor an alignment greater than 16 bytes. So
 * alignas(64) cannot be trusted. So we will need to takeover the memory allocation for the Node class
 * to guarantee this alignment.
 */

class alignas(64) Node {

  public:
    ///Number of values that can be stored in one node
    static constexpr unsigned capacity = 6;

  private:
    ///In case I get lucky some day and have a supercomputer with a cache line != 64
    static constexpr int cache_line_size = 64;

    ///Pointer to the "next level" of 7 contigous Nodes.
    Node* children_{nullptr};

    ///Top 16 bits will hold numValues in the node and lower 48 bits will be the pointer to the parent
    PackedPtr parent_;

    ///Our data
    int64_t vals_[capacity];

  public:
    Node* children() const {
        return children_;
    }

    Node* parent() const {
        return parent_.getPtr<Node>();
    }

    ///How many values do we currently have in the node
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
        for(NodeIdx_t i = 0; i <= capacity; i++) {
            Node* d = children_ + i;
            d->parent_.setPtr(this);
        }
    }

    val_t at(NodeIdx_t idx) const {
        ASSERT(idx >= 0 && idx < capacity);
        return vals_[idx];
    }

    /// If val already exists or was successfully inserted, returns {idx,true}
    /// else, returns {idx,false} where idx is the location where it should logically
    /// have been inserted were the node not already full
    std::tuple<NodeIdx_t,bool> insert(int64_t val) {
        bool found{false};
        NodeIdx_t idx{invalid_idx};

        std::tie(idx,found) = find(val);
        if(!found) {
            if(!isFull()) {
                for(NodeIdx_t i = numValues(); i > idx; i--) {
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
    /// returns {idx, true} if found else {idx, false} where
    /// idx is the logical position where the val should have been
    /// were it present in the node
    std::tuple<NodeIdx_t,bool> find(val_t val) const {
        NodeIdx_t idx = 0;
        for(idx = 0; idx < numValues() && vals_[idx] < val; idx++);
        if(idx == numValues() || vals_[idx] != val) {
            return {idx, false};
        } else {
            return {idx,true};
        }
    }
#else
    /// Use AVX2 instructions for optimized find
    /// As described in the design details, the performance gains by
    /// additionally using SIMD instructions to find the branching point
    /// are minimal compared to the gains by utilizing all the memory in a cache line
    std::tuple<NodeIdx_t,bool> find(int64_t val) const;

    /// When using AVX2 find, we will have to rely on a sentinel value in the node. So
    /// we need a constructor to fill in the sentinel value
    Node();
#endif

    /// We control the memory management for the Node class to ensure that we get memory
    /// aligned on a cache boundary
    void* operator new[](size_t size);
    void* operator new(size_t size);

    void operator delete(void* p);
    void operator delete[](void* p);
    ~Node();
};

#ifdef USE_AVX2

inline
Node::Node() {
    for(NodeIdx_t i = 0; i < capacity; i++) {
        //Fill in a sentinel value.
        vals_[i] = std::numeric_limits<int64_t>::max();
    }
}

inline
std::tuple<NodeIdx_t,bool> Node::find(val_t val) const {
    //We first look in the first 32 bytes. If we dont find the branching point there,
    //we look in the next 32 bytes.

    bool found{false};
    NodeIdx_t idx{0};
    constexpr int64_t maxint64 = std::numeric_limits<int64_t>::max();

    //Load the first 32 bytes
    __m256i valsp = _mm256_load_si256(reinterpret_cast<const __m256i*>(this));

    //Remember that the first 16 bytes are not really values and so we need an
    //approprite mask to mask them out
    __m256i targetp = {maxint64, maxint64,val,val};

    //Compare for greater than. We now run into one of the several annoying gaps in the SSE/AVX2 instruction set
    //Here, we have intrinsics only for > comparison, but not for >=. So we will have to detect the equality later
    //as seen below.
    __m256i maskgtp = _mm256_cmpgt_epi64(valsp,targetp);
    int mask = _mm256_movemask_epi8(maskgtp);

    if(mask != 0) {
        //we have found the value/branching point in the first 32 bytes of the cache line
        unsigned trailingZeroes = __builtin_ctz(static_cast<uint32_t>(mask));
        unsigned firstQuad = trailingZeroes / 8;
        ASSERT(firstQuad > 1);
        idx = firstQuad - 2;

        //Since we have compared for > , we need to check for equality. idx is the first location
        //that is greater than our value. So see if the the value at (idx-1) is equal to val
        found = idx > 0 ? vals_[idx-1] == val : false;
        idx = found ? idx-1 : idx;
    } else {
        //we need to look in the trailing 32 bytes of the cache line. We basically repeat the above again.
        valsp = _mm256_load_si256(reinterpret_cast<const __m256i*>(&vals_[2]));
        targetp = _mm256_set1_epi64x(val);
        maskgtp = _mm256_cmpgt_epi64(valsp,targetp);
        int mask = _mm256_movemask_epi8(maskgtp);

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
