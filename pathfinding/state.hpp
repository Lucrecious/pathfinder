#pragma once

#include "scenario.hpp"
#include <tuple>

namespace pathfinding {

struct State {
    State() : x(0), y(0), jump(0), scenario_meta(Scenario_None) { }

    inline static State create(const int x, const int y) {
        State state;
        state.x = x;
        state.y = y;

        return state;
    }

    int x;
    int y;

    int jump;

    int scenario_meta;

    inline void translate(const int x, const int y, State& translated) const {
        translated = *this;
        translated.x += x;
        translated.y += y;
    }

    State only_position() {
        State copy = *this;
        copy.jump = 0;
        return copy;
    }

    inline bool is_floor_scenario() const { return (scenario_meta & Scenario_OnFloor) == Scenario_OnFloor; }
    inline bool is_air_scenario() const { return (scenario_meta & Scenario_InAir) == Scenario_InAir; }
    inline bool is_ledge_hang_left() const { return (scenario_meta & Scenario_OnLedgeLeft) == Scenario_OnLedgeLeft; }
    inline bool is_ledge_hang_right() const { return (scenario_meta & Scenario_OnLedgeRight) == Scenario_OnLedgeRight; }
};

inline bool operator==(const State& state1, const State& state2) {
    return state1.x == state2.x && state1.y == state2.y && state1.jump == state2.jump;
}

inline bool operator!=(const State& state1, const State& state2) {
    return !(state1 == state2);
}

inline bool operator<(const State& state1, const State& state2) {
    return std::tie(state1.x, state1.y, state1.jump) < std::tie(state2.x, state2.y, state2.jump);
}

}

namespace std {
template<> struct hash<pathfinding::State> {
    typedef pathfinding::State argument_type;
    typedef std::size_t result_type;
    std::size_t operator()(const pathfinding::State& state) const noexcept {
        return std::hash<int>()(state.x ^ (state.y << 4) ^ (state.jump << 8));
    }
};
}
