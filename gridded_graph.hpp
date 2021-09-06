#pragma once

#include "scene/main/node.h"

#include "character_parameters.hpp"
#include "grid.hpp"
#include "pathfinding/graph.hpp"

class GriddedGraph : public Node {
    GDCLASS(GriddedGraph, Node)
    
    pathfinding::Graph _graph;

    Ref<Grid> _grid;

    void _add_masses(const Vector<Rect2>& rect);

protected:
    static void _bind_methods();

public:
    GriddedGraph() : _grid(RES()) { _grid.instance(); }

    void refresh_static_masses();

    void grid_set(Ref<Grid> grid)
    {
        _grid = grid;
        if (_grid.is_null()) {
            _grid.instance();
        }

        _change_notify("grid");
    }
    Ref<Grid> grid_get() const { return _grid; }

    Vector2 find_floor(Ref<CharacterParameters> character_parameters, Vector2 graph_position, int depth = 10) const;

    Vector2 world_to_graph(Vector2 world_position) const;
    Vector2 graph_to_world(Vector2 graph_position, bool use_middle) const;

    Vector2 world_units(Vector2 graph_units) const;
    Vector2 graph_units(Vector2 world_units) const;

    const pathfinding::Graph& graph() const { return _graph; }

    PoolVector2Array get_closest_free_cell_in_world_cover(Ref<CharacterParameters> character_parameters, Vector2 point, Array regions, bool prefer_floor = false) const;
};
    