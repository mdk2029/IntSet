#pragma once

#include <inttypes.h>
#include <tuple>
#include "kset_node.h"

namespace Kset {

using NodeIdx = uint16_t;

std::tuple<Node*, int, bool> find(Node* root, int64_t val);
std::tuple<Node*, int, bool> insert(Node* root, int64_t val);
std::tuple<Node*, int, int64_t> find_min(Node* node);
std::tuple<Node*, int, int64_t> successor(Node* node, int loc);

}

