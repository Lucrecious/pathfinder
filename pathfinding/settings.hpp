#pragma once

namespace pathfinding {

struct Settings {
    int max_jump_height;

    // The amount of vertical motion in the air before the character can move right
    unsigned int air_stride;

    unsigned int width;
    unsigned int height;

    bool ledge_hang;
};

}
