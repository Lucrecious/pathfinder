#pragma once

#include <unordered_set>
#include <string>

#include "graph.hpp"

namespace pathfinding {

struct Test {
    std::string tag;
    pathfinding::Graph graph;
    pathfinding::State start;
    pathfinding::State goal;
    pathfinding::Settings settings;
    std::unordered_set<pathfinding::State> expected_path;
};

}