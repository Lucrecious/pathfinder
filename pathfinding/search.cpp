#include "tools/queue.hpp"
#include "search.hpp"
#include <algorithm>

using namespace pathfinding;

inline int _heuristic(const Settings& settings, const State& state, const int goal_x, const int goal_y) {
    return abs(goal_x - state.x) + abs(goal_y - state.y);
}

bool pathfinding::search(
    const Graph& graph,
    const Settings& settings,
    const Region& region,
    State initial,
    const int goal_x, const int goal_y,
    std::vector<State>& path) {

    graph.contextualize(settings, initial);

    path.clear();

    bool found_path = false;

    std::unordered_map<State, State> came_from;
    std::unordered_map<State, int> cost_so_far;

    tool::priority_queue<State, int> frontier;
    frontier.put(initial, 0);


    came_from[initial] = initial;
    cost_so_far[initial] = 0;

    State neighbors[MAX_NEIGHBORS];

    State goal_state;

    // Search for shortest path
    while (!frontier.empty()) {
        State current = frontier.get();

        if (graph.bottom_row_contains(settings, current, goal_x, goal_y)) {
            goal_state = current;
            found_path = true;
            break;
        }

        int n = graph.neighbors(settings, current, neighbors);

        for (int i = 0; i < n; i++) {
            State next = neighbors[i];

            if (next.x < region.x) continue;
            if (next.y < region.y) continue;
            if (next.x >= region.x + region.w) continue;
            if (next.y >= region.y + region.h) continue;

            double new_cost = cost_so_far[current] + graph.cost(settings, current, next);
            if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                cost_so_far[next] = new_cost;
                double priority = new_cost + _heuristic(settings, next, goal_x, goal_y);
                frontier.put(next, priority);
                came_from[next] = current;
            }
        }
    }

    if (!found_path) { return false; }

    // Reconstruct path
    State current = goal_state;

    while (current != initial) {
        path.push_back(current);
        current = came_from[current];
    }

    path.push_back(initial);
    std::reverse(path.begin(), path.end());

    return true;
}

const State& _peek(const std::vector<State>& path, const int current, const int distance) {
    int peek_index = current + distance;
    if (peek_index < 0) {
        return path.at(0);
    }

    if (peek_index >= path.size()) {
        return path.at(path.size()-1);
    }

    return path.at(peek_index);
}

void pathfinding::filter(const Graph& graph, const Settings& settings, std::vector<State>& path)  {
    int size = path.size();
    int count = 0;

    State filtered[path.size()];

    for (int i = 0; i < size; i++) {
        const State& current = path.at(i);

        // Always add first
        if (i == 0) {
            filtered[count++] = current;
            continue;
        }

        // Always add last
        if (i == size - 1) {
            filtered[count++] = current;
            continue;
        }

        bool on_floor = current.is_floor_scenario() || graph.get_at(current.x, current.y) == CHARACTER_TILEKIND;
        bool in_air = current.is_air_scenario();
            

        // Floor before air
        if (on_floor && _peek(path, i, 1).is_air_scenario()) {
            filtered[count++] =  current;
            continue;
        }

        // Floor after air
        if (on_floor && _peek(path, i, -1).is_air_scenario()) { 
            filtered[count++] = current;
            continue;
        }

        // Air before floor and moving upwards (prevent bonking from the bottom)
        if (in_air && current.y < _peek(path, i, -1).y && _peek(path, i, 1).is_floor_scenario()) {
            filtered[count++] = current;
            continue;
        }

        // Air before floor and moving downwards (prevent bonking from the top)
        if (in_air && current.y < _peek(path, i, 1).y && _peek(path, i, -1).is_floor_scenario()) {
            filtered[count++] = current;
            continue;
        }
        
        //Jump Peaks
        if (in_air && current.y < _peek(path, i, -2).y && (current.y < _peek(path, i, 1).y)) {
            filtered[count++] = current;
            continue;
        }

        // Falling and moving around a corner
        {
            const State& before = _peek(path, i, -1);
            const State& after = _peek(path, i, 1);
            TileKind tile = graph.get_at(after.x, after.y - 1);
            if (in_air && current.y > before.y && current.y == after.y &&
                (tile == TileKind::FLOOR_TILEKIND || tile == TileKind::UNTRAVERSABLE_TILEKIND)) {
                    filtered[count++] = current;
                    continue;
                }

        }

        /* 
         * For now, the only possible way that two adjecent states can be diagonal
         * is with ledge hang. So I'm just checking keeping the "misaligned" adjecent states.
         */

        // Keep state directly after a ledge climb
        {
            const State& before = _peek(path, i, -1);
            if (current.x != before.x && current.y != before.y) {
                filtered[count++] = current;
                continue;
            }
        }

        // Keep state directly before a ledge climb
        {
            const State& after = _peek(path, i, 1);
            if (current.x != after.x && current.y != after.y) {
                filtered[count++] = current;
                continue;
            }
        }


    }

    path.resize(count);
    for (int i = 0; i < count ; i++) {
        path[i] = filtered[i];
    }
}
