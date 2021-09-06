#pragma once

#include <unordered_map>

#include "settings.hpp"
#include "state.hpp"

#define MAX_NEIGHBORS 16

namespace pathfinding {

struct Region {
    int x;
    int y;
    int w;
    int h;
};

enum TileKind {
    UNTRAVERSABLE_TILEKIND = 0,
    AIR_TILEKIND = 1,
    FLOOR_TILEKIND = 2,
    CHARACTER_TILEKIND = 3,
};

class Graph {
public:
    Graph() {}

    TileKind get_at(int32_t x, int32_t y) const;
    bool set_at(int32_t x, int32_t y, const TileKind kind);

    int neighbors(const Settings& settings, const State& state, State neighbors[MAX_NEIGHBORS]) const;

    int cost(const Settings& settings, const State& state, const State& next) const;

    void contextualize(const Settings& settings, State& state) const;

    inline void clear() { _grid.clear(); }

    inline size_t used_tile_count() const { return _grid.size(); }

    inline bool on_floor(const Settings& settings, int32_t x, int32_t y) const { return _is_on_floor(settings, State::create(x, y)); }

    inline bool fits(const Settings& settings, int32_t x, int32_t y) const { return _can_fit(settings, State::create(x, y)); }

    bool bottom_row_contains(const Settings& settings, const State& state, int32_t x, int32_t y) const;

    inline bool is_traversable_tile(int32_t x, int32_t y) const;

private:
    int _get_scenario(const Settings& settings, const State& state) const;
    bool _is_on_floor(const Settings& settings, const State& state) const;
    bool _can_fit(const Settings& settings, const State& state) const;
    bool _is_on_left_ledge(const Settings& settings, const State& state) const;
    bool _is_on_right_ledge(const Settings& settings, const State& state) const;

    inline bool _can_move_sideways_in_air(const Settings& settings, const State& state) const {
        return (state.jump % settings.air_stride) == 0;
    }

    inline int calculate_jump_limit(const Settings& settings) const {
        int air_stride = settings.air_stride;

        int detours = settings.max_jump_height > air_stride ?
            1 + (settings.max_jump_height - air_stride) / (air_stride - 1) : 0;

        return settings.max_jump_height + detours + 1;
    }

    bool _next_state(const Settings& settings, const State& state, State& next_state_with_position) const;

    inline void _right_of(const State& state, State& right) const { state.translate(1, 0, right); }
    inline void _left_of(const State& state, State& left) const { state.translate(-1, 0, left); }
    inline void _above(const State& state, State& above) const { state.translate(0, -1, above); }
    inline void _below(const State& state, State& below) const { state.translate(0, 1, below); }

    void _top_of_left_ledge(const Settings& settings, const State& state, State& over_ledge) const { state.translate(-settings.width, -settings.height, over_ledge); }
    void _top_of_right_ledge(const Settings& settings, const State& state, State& over_ledge) const { state.translate(settings.width, -settings.height, over_ledge); }

    std::unordered_map<int64_t, TileKind> _grid;
};

}