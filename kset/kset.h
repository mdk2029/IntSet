#pragma once

#include <inttypes.h>
#include <tuple>
#include <limits>
#include "kset_node.h"

/**
 * \ingroup Kset
 */

namespace Kset {

///Find val in tree rooted at root. Returns <position,true> if found else <potentialposition, false> if not found
std::tuple<Node*, NodeIdx_t, bool> find(Node* root, val_t val);

///Find val in tree rooted at root. Returns position at which val was inserted. Always succeeds
std::tuple<Node*, NodeIdx_t, bool> insert(Node* root, val_t val);

///Find min element of tree rooted at node. Used mostly by successor
std::tuple<Node*, NodeIdx_t, val_t> find_min(Node* node);

///Find successor element
std::tuple<Node*, NodeIdx_t, val_t> successor(Node* node, NodeIdx_t loc);

/// Todo
/// deletion - Best implemented lazily. We have 16 free bits in the children_ ptr and those can be used
/// to keep track of which elements are alive and which are deleted in a node

}

