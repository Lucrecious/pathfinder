#pragma once

#include <vector>

#include "settings.hpp"
#include "state.hpp"
#include "graph.hpp"

namespace pathfinding {

void filter(const Graph& graph, const Settings& settings, std::vector<State>& path);

bool search(
    const Graph& graph,
    const Settings& settings,
    const Region& region,
    State initial,
    const int goal_x, const int goal_y,
    std::vector<State>& path);

}