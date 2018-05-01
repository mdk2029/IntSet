#pragma once

#include <inttypes.h>
#include <tuple>
#include "kset_node.h"

namespace Kset {

std::tuple<Node*, int, bool> find(Node* node, int64_t val);
std::tuple<Node*, int, bool> insert(Node* node, int64_t val);
std::tuple<Node*, int> successor(Node* node, int loc);

}

