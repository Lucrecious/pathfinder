#pragma once

#include "core/resource.h"

struct Grid : public Resource {
    GDCLASS(Grid, Resource)

    int _step_x;
    int _step_y;
    
protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("snapped", "vector"), &Grid::snapped);
        ClassDB::bind_method(D_METHOD("gridded", "vector"), &Grid::gridded);
        ClassDB::bind_method(D_METHOD("step"), &Grid::step);
        
        ClassDB::bind_method(D_METHOD("set_step_x", "value"), &Grid::set_step_x);
        ClassDB::bind_method(D_METHOD("get_step_x"), &Grid::get_step_x);

        ClassDB::bind_method(D_METHOD("set_step_y", "value"), &Grid::set_step_y);
        ClassDB::bind_method(D_METHOD("get_step_y"), &Grid::get_step_y);
        
        ADD_PROPERTY(PropertyInfo(Variant::INT, "step_x", PROPERTY_HINT_RANGE, "1,99999,1"), "set_step_x", "get_step_x");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "step_y", PROPERTY_HINT_RANGE, "1,99999,1"), "set_step_y", "get_step_y");
    }

public:
    inline Grid() {
        _step_x = 1;
        _step_y = 1;
    }

    inline Vector2 snapped(Vector2 v) const { return v.snapped(Vector2(_step_x, _step_y)); }
    inline Vector2 gridded(Vector2 v) const { return Vector2(floor(v.x / _step_x), floor(v.y / _step_y)); }
    inline Vector2 step() const { return Vector2(_step_x, _step_y); }

    inline void set_step_x(int value) { _step_x = value; }
    inline int get_step_x() const { return _step_x; }
    
    inline void set_step_y(int value) { _step_y = value; }
    inline int get_step_y() const { return _step_y; }
};