#pragma once

#include "scene/main/node.h"

#include "character_parameters.hpp"
#include "grid.hpp"
#include "gridded_graph.hpp"
#include "tools/threadpool.hpp"
#include "pathfinding/graph.hpp"

#include "core/map.h"

class Pathfinder : public Node {
    GDCLASS(Pathfinder, Node);

    NodePath _initial_graph_path;
    void _initial_graph_path_set(NodePath graph_path);
    NodePath _initial_graph_path_get() const;

    GriddedGraph* _graph;
    void _graph_set(GriddedGraph* graph);
    GriddedGraph* _graph_get() const;

    bool _filtered;
    void _filtered_set(bool value);
    bool _filtered_get() const;

    int _max_concurrency;
    ThreadPool* _pool;
    unsigned int _id_counter;

    std::mutex _lock;
    std::unordered_map<int, std::pair<Object*, String>> _callbacks;
    std::vector<std::pair<int, Dictionary>> _results;

    void _compute_path_async(
        int id,
        const pathfinding::Graph& graph,
        const pathfinding::Settings& settings,
        const pathfinding::Region& region,
        const pathfinding::State& initial,
        const int goal_x, const int goal_y,
        Object* object, String method);
        

protected:
    static void _bind_methods();

public:
    Pathfinder();
    ~Pathfinder();

    void _notification(int what);

    void _do_callbacks();

    int compute_path(Vector2 initial, Vector2 goal, Ref<CharacterParameters> character_parameters, Rect2 region, Array dynamic_masses_world, Object* object, String method);
    void cancel(int id);

    enum Scenario {
        None = pathfinding::Scenario_None,
        OnFloor = pathfinding::Scenario_OnFloor,
        InAir = pathfinding::Scenario_InAir,
        LedgeHangOnLeft = pathfinding::Scenario_OnLedgeLeft,
        LedgeHangOnRight = pathfinding::Scenario_OnLedgeRight,
    };

};

VARIANT_ENUM_CAST(Pathfinder::Scenario);
