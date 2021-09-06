#include "graph.hpp"

using namespace pathfinding;

TileKind Graph::get_at(int32_t x, int32_t y) const {
    int64_t index = ((int64_t)y << 32) | x;
    auto it = _grid.find(index);

    if (it == _grid.end()) return AIR_TILEKIND;
    return it->second;
}

bool Graph::set_at(int32_t x, int32_t y, const TileKind kind) {
    _grid[((int64_t)y << 32) | x] = kind;
    return true;
}

void Graph::contextualize(const Settings& settings, State& state) const {
    state.scenario_meta = _get_scenario(settings, state);
}

int Graph::neighbors(const Settings& settings, const State& state, State neighbors[MAX_NEIGHBORS]) const {
    int kind = get_at(state.x, state.y);
    if (kind == UNTRAVERSABLE_TILEKIND) return 0;

    int count = 0;

    int num_positions = 4;
    _right_of(state, neighbors[0]);
    _left_of(state, neighbors[1]);
    _above(state, neighbors[2]);
    _below(state, neighbors[3]);

    if (settings.ledge_hang) {
        num_positions += 2;
        _top_of_left_ledge(settings, state, neighbors[4]);
        _top_of_right_ledge(settings, state, neighbors[5]);
    }

    for (int i = 0; i < num_positions; i++) {
        State& adjecent = neighbors[i];
        if (!_can_fit(settings, adjecent)) continue;

        contextualize(settings, adjecent);
        if (!_next_state(settings, state, adjecent)) continue;

        neighbors[count++] = adjecent;
    }

    return count;
}

int Graph::cost(const Settings& settings, const State& state, const State& next) const {
    int base = 0;

    // Only ledge hang when the jump can't be reached by a regular jump
    if (next.x != state.x && next.y != state.y) base += settings.max_jump_height * 3;

    // Moving up takes up the "most" energy
    else if (next.y < state.y) base += 3;

    // Moving down takes up the least energy
    else if (next.y > state.y) base += 1;
    
    // Everything else is neutral
    else base += 2;

    // Other characters are hard to move through
    if (get_at(next.x, next.y) == CHARACTER_TILEKIND) {
        base += 70;
    }

    return base;
}

bool Graph::bottom_row_contains(const Settings& settings, const State& state, int32_t x, int32_t y) const {
    return state.y == y && (x >= state.x && x < state.x + settings.width);
}

bool Graph::is_traversable_tile(int32_t x, int32_t y) const {
    auto kind = get_at(x, y);
    return kind == AIR_TILEKIND || kind == CHARACTER_TILEKIND;
}

bool Graph::_next_state(const Settings& settings, const State& state, State& next_state_with_position) const {
    /*
     * Ledge Hanging Note:
     * To handle ledge hang there is no need for any other special logic aside from the if-statement below.
     * Conveniently, ledge hangs can only come from an air scenario and end up in a floor scenario so it can
     * all be handled before the regular logic...
     */
    if (settings.ledge_hang) {
        if (state.x != next_state_with_position.x && state.y != next_state_with_position.y) {
            if (next_state_with_position.x < state.x && !state.is_ledge_hang_left()) return false;
            if (next_state_with_position.x > state.x && !state.is_ledge_hang_right()) return false;

            if (!next_state_with_position.is_floor_scenario()) return false;

            next_state_with_position.jump = 0;
            return true;
        }
    }

    // Handle Regular Floor Scenarios
    int jump_limit = 0;
    int air_stride = settings.air_stride;
    bool can_move_up = false;


    if (settings.max_jump_height) {
        jump_limit = calculate_jump_limit(settings);
        can_move_up = jump_limit - state.jump > 0;
    }

    if (state.is_floor_scenario()) {
        if (next_state_with_position.is_floor_scenario()) {
            next_state_with_position.jump = 0;
            return true;
        }

        if (next_state_with_position.is_air_scenario()) {
            if (next_state_with_position.y < state.y && !can_move_up) return false;

            if (next_state_with_position.x != state.x) {
                // Must fall after before being able to move horizontally
                next_state_with_position.jump = jump_limit % air_stride == 0 ?
                    jump_limit + 1 : jump_limit;
                
                return true;
            }

            // Must be moving vertically
            if (next_state_with_position.y < state.y) {
                next_state_with_position.jump = 2;
                return true;
            }

            // Must be moving down
            next_state_with_position.jump = jump_limit;
            return true;
        }

        return false;
    }

    if (state.is_air_scenario()) {
        if (next_state_with_position.y < state.y && !can_move_up) return false;
        
        bool can_move_sideways = (state.jump % air_stride) == 0;

        if (next_state_with_position.x != state.x && !can_move_sideways) return false;

        if (next_state_with_position.is_floor_scenario()){
            next_state_with_position.jump = 0;
            return true;
        }

        if (next_state_with_position.is_air_scenario()) {
            if (can_move_sideways && next_state_with_position.x != state.x) {
                next_state_with_position.jump = state.jump + 1;
                return true;
            }

            if (next_state_with_position.y > state.y && can_move_up) {
                next_state_with_position.jump = jump_limit;
                return true;
            }

            // Must be moving vertically
            if (can_move_sideways) {
                // Skips the horizontal detour, so must add 2
                next_state_with_position.jump = state.jump + 2;
                return true;
            }

            next_state_with_position.jump = state.jump + 1;
            return true;
        }

        return false;
    }

    return false;
}

int Graph::_get_scenario(const Settings& settings, const State& state) const {
    if (_is_on_floor(settings, state)) return Scenario_OnFloor;

    int scenario = Scenario_InAir;
    if (settings.ledge_hang) {
        if (_is_on_left_ledge(settings, state)) {
            scenario = scenario | Scenario_OnLedgeLeft;
        }

        if (_is_on_right_ledge(settings, state)) {
            scenario = scenario | Scenario_OnLedgeRight;
        }
    }

    return scenario;
}

bool Graph::_is_on_floor(const Settings& settings, const State& state) const {
    TileKind kind;
    for (int i = 0; i < settings.width; i++) {
        kind = get_at(state.x + i, state.y + 1);
        if (kind == FLOOR_TILEKIND) return true;
    }

    return false;
}

bool Graph::_can_fit(const Settings& settings, const State& state) const {
    TileKind kind;
    for (int i = 0; i < settings.width; i++) {
        for (int j = 0; j < settings.height; j++) {
            kind = get_at(state.x + i, state.y - j);
            if (kind == FLOOR_TILEKIND || kind == UNTRAVERSABLE_TILEKIND) return false;
        }
    }

    return true;
}

bool Graph::_is_on_left_ledge(const Settings& settings, const State& state) const {
    State top_left = state;
    top_left.y += -(settings.height - 1);

    bool at_left_ledge = get_at(top_left.x - 1, top_left.y) == FLOOR_TILEKIND &&
                         is_traversable_tile(top_left.x - 1, top_left.y - 1) &&
                         is_traversable_tile(top_left.x, top_left.y - 1);

    return at_left_ledge;
}

bool Graph::_is_on_right_ledge(const Settings& settings, const State& state) const {
    State top_right = state;
    top_right.x += settings.width - 1;
    top_right.y += -(settings.height - 1);

    bool at_right_ledge = get_at(top_right.x + 1, top_right.y) == FLOOR_TILEKIND &&
                          is_traversable_tile(top_right.x + 1, top_right.y - 1) &&
                          is_traversable_tile(top_right.x, top_right.y - 1);
    
    return at_right_ledge;
}


