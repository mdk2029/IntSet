#include "kset.h"

namespace Kset {


void* Node::operator new[](size_t size) {
    void* memptr = nullptr;
    if(int ret = posix_memalign(&memptr, cache_line_size, size) != 0) {
        std::cerr << "posix_memalign failed due to : " << std::strerror(ret) << std::endl;
        throw std::bad_alloc();
    }

    return memptr;
}

void Node::operator delete[](void* p) {
    free(p);
}

void* Node::operator new(size_t size) {
    void* memptr = nullptr;
    if(int ret = posix_memalign(&memptr, cache_line_size, size) != 0) {
        std::cerr << "posix_memalign failed due to : " << std::strerror(ret) << std::endl;
        throw std::bad_alloc();
    }

    return memptr;
}

void Node::operator delete(void* p) {
    free(p);
}

Node::~Node() {
    delete[] children_;
}


//returns destination node and position within the node
std::tuple<Node*, int, bool> find(Node* node, int64_t val) {

    ASSERT_IMPLIES( (node->numValues_ < node->capacity), !node->children_ );

    int idx{-1};
    bool found{false};
    std::tie(idx,found) = node->find(val);

    if(!found) {
        //find in descendent block
        //<descendent ptr><8 bytes reserved><int0, int1, ...., int11>
        //<so if say int1 is greater than our val,then we need the block for
        //vals between int0 and int1 which would be at descendentptr + 1
        if(node->children_) {
            Node* descendent = node->children_ + idx;
            return find(descendent, val);
        }
    }
    return {node,idx,found};
}

std::tuple<Node*, int, bool> insert(Node* node, int64_t val)  {

    ASSERT_IMPLIES( (node->numValues_ < node->capacity), !node->children_ );

    int idx{-1};
    bool inserted{false};
    bool found{false};

    std::tie(node,idx,found) = find(node,val);

    if(!found) {
        std::tie(idx,inserted) = node->insert(val);
        if(!inserted) {
            node->expand();
            node = node->children_ + idx;
            std::tie(idx,inserted) = node->insert(val);
        }
        ASSERT(inserted);
    }
    return {node,idx,inserted};
}

}
