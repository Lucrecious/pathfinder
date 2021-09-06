#include "gridded_graph.hpp"

#include <algorithm>
#include <vector>

#include "core/script_language.h"

struct less_than_closest_point {
    const pathfinding::Graph& _graph;
    const pathfinding::Settings& _settings;
    Vector2 _point;
    bool _prefer_floors;
    
    less_than_closest_point(const pathfinding::Graph& graph, const pathfinding::Settings& settings, Vector2 point, bool prefer_floors)
    : _graph(graph), _settings(settings), _point(point), _prefer_floors(prefer_floors) {}

    inline bool operator()(const Vector2& v1, const Vector2& v2) {
        if (_prefer_floors) {
            bool v1_on_floor = _graph.on_floor(_settings, v1.x, v1.y);
            bool v2_on_floor = _graph.on_floor(_settings, v2.x, v2.y);
            if (v1_on_floor != v2_on_floor) return v1_on_floor;
        }

        float delta1 = (v1 - _point).abs().dot(Vector2(1, 1));
        float delta2 = (v2 - _point).abs().dot(Vector2(1, 1));
        return delta1 < delta2;
    }
};

void GriddedGraph::_bind_methods() {
    ClassDB::bind_method(D_METHOD("grid_set", "grid"), &GriddedGraph::grid_set);
    ClassDB::bind_method(D_METHOD("grid_get"), &GriddedGraph::grid_get);

    ClassDB::bind_method(D_METHOD("refresh_static_masses"), &GriddedGraph::refresh_static_masses);

    ClassDB::bind_method(D_METHOD("find_floor", "character_parameters", "graph_position", "depth"), &GriddedGraph::find_floor, DEFVAL(10));

    ClassDB::bind_method(D_METHOD("world_to_graph", "world_position"), &GriddedGraph::world_to_graph);
    ClassDB::bind_method(D_METHOD("graph_to_world", "graph_position", "use_middle"), &GriddedGraph::graph_to_world);

    ClassDB::bind_method(D_METHOD("world_units", "graph_units"), &GriddedGraph::world_units);
    ClassDB::bind_method(D_METHOD("graph_units", "world_units"), &GriddedGraph::graph_units);

    ClassDB::bind_method(D_METHOD("get_closest_free_cell_in_world_cover", "character_parameters", "point", "regions", "prefer_floors"), &GriddedGraph::get_closest_free_cell_in_world_cover, DEFVAL(false));

   	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grid", PROPERTY_HINT_RESOURCE_TYPE, "Grid"), "grid_set", "grid_get");
}

Vector2 GriddedGraph::find_floor(Ref<CharacterParameters> character_parameters, Vector2 graph_position, int depth) const {
    graph_position = graph_position.snapped(Vector2(1, 1));
    int x = (int)graph_position.x;
    int y = (int)graph_position.y;
    for (int i = 0; i < depth; i++) {
        if (!_graph.fits(character_parameters->settings(), x, y + i)) continue;
        if (_graph.on_floor(character_parameters->settings(), x, y + i)) return graph_position;
    }

    return Vector2(NAN, NAN);
}

Vector2 GriddedGraph::world_to_graph(Vector2 world_position) const {
    return grid_get()->gridded(world_position);
}

Vector2 GriddedGraph::graph_to_world(Vector2 graph_position, bool use_middle) const {
    return grid_get()->step() * graph_position + (use_middle ? grid_get()->step() / 2.0 : Vector2());
}

Vector2 GriddedGraph::world_units(Vector2 graph_units) const {
    return graph_units * grid_get()->step();
}

Vector2 GriddedGraph::graph_units(Vector2 world_units) const {
    return world_units / grid_get()->step();
}

PoolVector2Array GriddedGraph::get_closest_free_cell_in_world_cover(Ref<CharacterParameters> character_parameters, Vector2 point, Array regions, bool prefer_floors) const {
    std::vector<Vector2> free_cells;

    for (int i = 0; i < regions.size(); i++) {
        auto rect = (Rect2)regions[i];
        Vector2 rect_pos = grid_get()->gridded(rect.position);
        Vector2 rect_size = (grid_get()->gridded(rect.position + rect.size) - rect_pos) + Vector2(1, 1);
        rect.position = rect_pos;
        rect.size = rect_size;

        for (int r = 0; r < rect.size.y; r++) {
            for (int c = 0; c < rect.size.x; c++) {
                Vector2 cell(rect.position.x + c, rect.position.y + r);
                if (!_graph.fits(character_parameters->settings(), (int32_t)cell.x, (int32_t)cell.y)) continue;
                free_cells.push_back(cell);
            }
        }
    }

    std::sort(free_cells.begin(), free_cells.end(), less_than_closest_point(_graph, character_parameters->settings(), grid_get()->gridded(point), prefer_floors));
    free_cells.erase(std::unique(free_cells.begin(), free_cells.end()), free_cells.end());

    PoolVector2Array ret_free_cells;
    for (int i = 0; i < free_cells.size(); i++) {
        ret_free_cells.append(free_cells[i]);
    }

    return ret_free_cells;
}

void GriddedGraph::_add_masses(const Vector<Rect2>& masses) {
    if (_grid.is_null()) return;

    _graph.clear();

    for (int i = 0; i < masses.size(); i++) {
        Rect2 rect = masses[i];

        rect.position = _grid->gridded(_grid->snapped(rect.position));
        rect.size = _grid->gridded(_grid->snapped(rect.size));

        for (int y = 0; y < rect.size.height; y++) {
            for (int x = 0; x < rect.size.width; x++) {
                int index_x = rect.position.x + x;
                int index_y = rect.position.y + y;

                _graph.set_at(index_x, index_y, pathfinding::FLOOR_TILEKIND);
            }
        }
    }
}

void GriddedGraph::refresh_static_masses() {
    if (get_script_instance() == nullptr) return;
    if (!get_script_instance()->has_method("_refresh_static_masses")) return;

    Variant ret = get_script_instance()->call("_refresh_static_masses");
    if (ret.get_type() != Variant::ARRAY) return;

    Vector<Rect2> landmasses;

    Array arr = (Array)ret;

    for (int i = 0; i < arr.size(); i++) {
        Variant rect = arr[i];
        if (rect.get_type() != Variant::RECT2) continue;
        landmasses.push_back(rect);
    }

    _add_masses(landmasses);
}
