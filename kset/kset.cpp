#include "kset.h"
#include "errors.h"

namespace Kset {

//returns destination node and position within the node
std::tuple<Node*, int, bool> find(Node* node, int64_t val) {

    ASSERT_IMPLIES( (node->numValues() < node->capacity), !node->children() );

    int idx{-1};
    bool found{false};
    std::tie(idx,found) = node->find(val);

    if(!found) {
        //find in descendent block
        //<descendent ptr><8 bytes reserved><int0, int1, ...., int11>
        //<so if say int1 is greater than our val,then we need the block for
        //vals between int0 and int1 which would be at descendentptr + 1
        if(node->children()) {
            Node* descendent = node->children() + idx;
            return find(descendent, val);
        }
    }
    return {node,idx,found};
}

std::tuple<Node*, int, bool> insert(Node* node, int64_t val)  {

    ASSERT_IMPLIES( (node->numValues() < node->capacity), !node->children() );

    int idx{-1};
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

}
