#include "kset.h"
#include "errors.h"

namespace Kset {

std::tuple<Node*, NodeIdx_t, bool> find(Node* node, val_t val) {

    ASSERT_IMPLIES( (node->numValues() < node->capacity), !node->children() );

    NodeIdx_t idx{invalid_idx};
    bool found{false};
    std::tie(idx,found) = node->find(val);

    if(!found) {
        //find in descendent block
        //<descendent ptr><parent ptr><int0, int1, ...., int5>
        //<so if say int1 is greater than our val,then we need the block for
        //vals between int0 and int1 which would be at descendentptr + 1
        if(node->children()) {
            Node* descendent = node->children() + idx;
            return find(descendent, val);
        }
    }
    return {node,idx,found};
}

std::tuple<Node*, NodeIdx_t, bool> insert(Node* node, val_t val)  {

    ASSERT_IMPLIES( (node->numValues() < node->capacity), !node->children() );

    NodeIdx_t idx{invalid_idx};
    bool inserted{false};
    bool found{false};

    std::tie(node,idx,found) = find(node,val);

    if(!found) {
        std::tie(idx,inserted) = node->insert(val);
        if(!inserted) {
            node->expand();
            node = node->children() + idx;
            std::tie(idx,inserted) = node->insert(val);
        }
        ASSERT(inserted);
    }
    return {node,idx,inserted};
}

std::tuple<Node*,NodeIdx_t,val_t> find_min(Node* node) {

    ASSERT(node->numValues() > 0);

    while(node->children() && node->children()->numValues()) {
        node = node->children();
    }

    return {node, 0, node->at(0)};
}

std::tuple<Node*,NodeIdx_t,val_t> successor(Node* node, NodeIdx_t loc) {

    ASSERT(node->numValues() > loc);

    //First look in the potential child node
    if(node->children()) {
        Node* potentialDescendent = node->children() + loc + 1;
        if(potentialDescendent->numValues() > 0) {
            return find_min(potentialDescendent);
        }
    }

    //Next look in the same node
    if(node->numValues() > loc+1) {
        return {node, loc+1, node->at(loc+1)};
    }

    //will have to go up to the parent. May have to keep going up the chain
    //till we find an ancestor with a value greater than our val
    val_t val = node->at(loc);
    Node* parent = node->parent();
    while(parent) {
        ASSERT(parent->numValues());
        for(NodeIdx_t i = 0; i < parent->numValues(); i++) {
            if(parent->at(i) > val) {
                return {parent,i,parent->at(i)};
            }
        }
        parent = parent->parent();
    }
    return {nullptr, invalid_idx, -1};
}

}
