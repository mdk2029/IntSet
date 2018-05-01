#include "gtest/gtest.h"
#include <kset/kset.h>
#include <kset/packed_ptr.h>
#include <boost/scope_exit.hpp>
#include <memory>

namespace Kset {

static const unsigned max_values_in_node = Kset::Node::capacity;

GTEST_TEST(NodeTest, construction) {
    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();
    ASSERT_EQ(sizeof(Node), 64);
    ASSERT_EQ(n->children(), nullptr);
    ASSERT_EQ(n->numValues(), 0);
    ASSERT_EQ((int64_t)n % 64, 0);

#ifdef USE_AVX2
    for(unsigned i = 0; i < max_values_in_node; i++) {
        ASSERT_EQ(n->vals_[i] , std::numeric_limits<int64_t>::max());
    }
#endif

}

GTEST_TEST(NodeTest, basic_insertion) {
    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();

    for(int i = max_values_in_node-1; i >= 0; i--) {
        int idx{0};
        bool inserted{false};
        std::tie(idx,inserted) = n->insert(static_cast<int64_t>(i));
        ASSERT_TRUE(inserted);
    }

    for(unsigned i = 0; i < max_values_in_node; i++) {
        ASSERT_EQ(n->vals_[i] , static_cast<int64_t>(i));
    }
}

GTEST_TEST(NodeTest, local_insertion) {
    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();

    for(unsigned i = 0; i < max_values_in_node; i++) {
        bool inserted{false};
        int idx{-1};
        std::tie(idx,inserted) = n->insert(i * 100);
        ASSERT_TRUE(inserted);
        ASSERT_EQ(idx,(int)i);
    }

    ASSERT_EQ(n->children(), nullptr);
    ASSERT_EQ(n->numValues(), max_values_in_node);

    int idx{0};
    bool inserted{true};

    std::tie(idx,inserted) = n->insert(1000);

    ASSERT_EQ(inserted,false);

    bool found{false};
    for(unsigned i = 0; i < max_values_in_node; i++) {
        std::tie(idx,found) = n->find(i*100);
        ASSERT_EQ(found,true);
        ASSERT_EQ(idx, i);
    }

    std::tie(idx,found) = n->find(150);
    ASSERT_EQ(found, false);
    ASSERT_EQ(idx, 2);

    std::tie(idx,found) = n->find(1000000);
    ASSERT_EQ(found, false);
    ASSERT_EQ(idx, max_values_in_node);

}

GTEST_TEST(NodeTest, insertion) {
    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();

    for(unsigned i = 0; i < max_values_in_node; i++) {
        n->insert(i * 100);
    }

    Kset::Node* dest{nullptr};
    int idx{-1};
    bool inserted{false};

    std::tie(dest,idx,inserted) = insert(n,100);
    ASSERT_FALSE(inserted);

    std::tie(dest,idx,inserted) = insert(n,50);
    ASSERT_TRUE(inserted);
    ASSERT_EQ(dest, n->children()+1);
    ASSERT_EQ(idx, 0);

    ASSERT_NE(n->children(),nullptr);
    ASSERT_EQ(n->children()->numValues(), 0);

    ASSERT_EQ(dest->children(), nullptr);
    ASSERT_EQ(dest->numValues(), 1);

}

GTEST_TEST(NodeTest, find) {
    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();

    for(unsigned i = 0; i < max_values_in_node; i++) {
        n->insert(i * 100);
    }

    ASSERT_TRUE(n->isFull());

    Kset::Node* dest{nullptr};
    int idx{-1};
    bool inserted{false};
    std::tie(dest,idx,inserted) = insert(n,50);
    ASSERT_TRUE(inserted);

    Kset::Node* node{nullptr};
    bool found{false};
    std::tie(node,idx,found) = find(n,50);
    ASSERT_EQ(found,true);
    ASSERT_EQ(node->numValues(), 1);

    std::tie(node,idx,found) = find(n,55);
    ASSERT_EQ(found,false);
    ASSERT_EQ(idx, 1);
    ASSERT_EQ(node->numValues(), 1);
}

GTEST_TEST(NodeTest, insertion_find) {
    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();
    std::set<int64_t> insertedVals;

    bool inserted{false};
    const int size = 1000000;
    for(unsigned i = 0; i < size; i++) {
        int64_t val = std::rand() % size;
        std::tie(std::ignore, std::ignore, inserted) = insert(n,val);
        if(inserted) {
            insertedVals.insert(val);
        }
    }

    bool found{false};
    for(unsigned i = 0; i < size; i++) {
        bool isInInsertedVals = insertedVals.count(i);
        std::tie(std::ignore,std::ignore,found) = find(n,i);
        ASSERT_EQ(isInInsertedVals,found);
    }
}

GTEST_TEST(PackedWord, PackedWordTest) {
    Kset::PackedPtr ptr;

    ASSERT_EQ(ptr.packedWord(), 0);
    ptr.setData(0x42);
    ASSERT_EQ(ptr.getData(), 0x42);
    ASSERT_EQ(ptr.packedWord(), 0x0042000000000000);

    auto un = std::make_unique<Kset::Node>();
    Kset::Node* n = un.get();
    ptr.setPtr(n);
    ASSERT_EQ(ptr.getData(), 0x42);
    ASSERT_EQ(ptr.getPtr<Kset::Node>(), n);
}

}

