#pragma once

#include "core/resource.h"
#include "core/math/math_funcs.h"

#include "pathfinding/settings.hpp"

class CharacterParameters : public Resource{
    GDCLASS(CharacterParameters, Resource);

private:
    pathfinding::Settings _settings;

    void _max_jump_height_set(int max_jump_height) { _settings.max_jump_height = Math::abs(max_jump_height); }
    int _max_jump_height_get() { return _settings.max_jump_height; }
    
    void _air_stride_set(int air_stride) { _settings.air_stride = Math::abs(air_stride); }
    int _air_stride_get() { return _settings.air_stride; }
    
    void _width_set(int width) { _settings.width = Math::abs(width); }
    int _width_get() { return _settings.width; }

    void _height_set(int height) { _settings.height = Math::abs(height); }
    int _height_get() { return _settings.height; }

    void _ledge_hang_set(bool value) { _settings.ledge_hang = value; }
    bool _ledge_hang_get() {  return _settings.ledge_hang; }

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("max_jump_height_set", "value"), &CharacterParameters::_max_jump_height_set);
        ClassDB::bind_method(D_METHOD("max_jump_height_get"), &CharacterParameters::_max_jump_height_get);
        
        ClassDB::bind_method(D_METHOD("air_stride_set", "value"), &CharacterParameters::_air_stride_set);
        ClassDB::bind_method(D_METHOD("air_stride_get"), &CharacterParameters::_air_stride_get);

        ClassDB::bind_method(D_METHOD("width_set", "value"), &CharacterParameters::_width_set);
        ClassDB::bind_method(D_METHOD("width_get"), &CharacterParameters::_width_get);

        ClassDB::bind_method(D_METHOD("height_set", "value"), &CharacterParameters::_height_set);
        ClassDB::bind_method(D_METHOD("height_get"), &CharacterParameters::_height_get);

        ClassDB::bind_method(D_METHOD("ledge_hang_set", "value"), &CharacterParameters::_ledge_hang_set);
        ClassDB::bind_method(D_METHOD("ledge_hang_get"), &CharacterParameters::_ledge_hang_get);

        ADD_PROPERTY(PropertyInfo(Variant::INT, "max_jump_height", PROPERTY_HINT_RANGE, "0"), "max_jump_height_set", "max_jump_height_get");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "air_stride", PROPERTY_HINT_RANGE, "1"), "air_stride_set", "air_stride_get");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "width", PROPERTY_HINT_RANGE, "1"), "width_set", "width_get");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "1"), "height_set", "height_get");

        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ledge_hang", PROPERTY_HINT_NONE), "ledge_hang_set", "ledge_hang_get");
    }

public:
    CharacterParameters() {
        _settings.max_jump_height = 0;
        _settings.air_stride = 1;
        _settings.width = 1;
        _settings.height = 1;
        _settings.ledge_hang = false;
    }

    const pathfinding::Settings& settings() { return _settings; }
};