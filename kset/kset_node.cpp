#include "kset_node.h"
#include <iostream>
#include <cstring>

namespace Kset {

/// We use posix_memalign to guarantee alignment.
/// Todo: Use a pool of nodes
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

}
