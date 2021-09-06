#include "pathfinder.hpp"

#include "core/method_bind_ext.gen.inc"

#include "pathfinding/search.hpp"

Pathfinder::Pathfinder() {
    _initial_graph_path = NodePath();
    _graph = nullptr;
    _filtered = true;
    _max_concurrency = 4;
}

Pathfinder::~Pathfinder() {}

void Pathfinder::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initial_graph_path_set", "initial_graph_path"), &Pathfinder::_initial_graph_path_set);
    ClassDB::bind_method(D_METHOD("initial_graph_path_get"), &Pathfinder::_initial_graph_path_get);

    ClassDB::bind_method(D_METHOD("filtered_set", "value"), &Pathfinder::_filtered_set);
    ClassDB::bind_method(D_METHOD("filtered_get"), &Pathfinder::_filtered_get);

    ClassDB::bind_method(D_METHOD("_do_callbacks"), &Pathfinder::_do_callbacks);

    ClassDB::bind_method(D_METHOD("compute_path",
        "initial", "goal", "character_parameters", "region", "dynamic_masses", "source", "callback"), &Pathfinder::compute_path);
    ClassDB::bind_method(D_METHOD("cancel", "id"), &Pathfinder::cancel);

   	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "initial_graph_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "GriddedGraph"), "initial_graph_path_set", "initial_graph_path_get");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "filtered"), "filtered_set", "filtered_get");

    BIND_ENUM_CONSTANT(None);
    BIND_ENUM_CONSTANT(OnFloor);
    BIND_ENUM_CONSTANT(InAir);
    BIND_ENUM_CONSTANT(LedgeHangOnLeft);
    BIND_ENUM_CONSTANT(LedgeHangOnRight);
}

void Pathfinder::_do_callbacks() {
    std::vector<std::pair<int, Dictionary>> results;
    {
        std::unique_lock<std::mutex> unique_lock(_lock);
        results = _results;
        _results.clear();
    }

    for (int i = 0; i < results.size(); i++) {
        unsigned int id = results[i].first;
        auto it = _callbacks.find(id);
        if (it == _callbacks.end()) continue;
        Object* obj = it->second.first;
        String method = it->second.second;

        _callbacks.erase(it);

        if (!obj) continue;
        if (!obj->has_method(method)) continue;

        obj->call(method, results[i].second);
    }
}

void Pathfinder::_initial_graph_path_set(NodePath initial_graph_path) {
    _initial_graph_path = initial_graph_path;
}

NodePath Pathfinder::_initial_graph_path_get() const {
    return _initial_graph_path;
}

void Pathfinder::_graph_set(GriddedGraph* graph) {
    _graph = graph;
}

GriddedGraph* Pathfinder::_graph_get() const {
    return _graph;
}

void Pathfinder::_filtered_set(bool value) {
    _filtered = value;
}

bool Pathfinder::_filtered_get() const {
    return _filtered;
}

void Pathfinder::_notification(int what) {
    switch (what) {
        case NOTIFICATION_READY:
        {
            _graph = Object::cast_to<GriddedGraph>(get_node_or_null(_initial_graph_path));
            break;
        }
        case NOTIFICATION_ENTER_TREE:
        {
            _pool = new ThreadPool(_max_concurrency);
            get_tree()->connect("idle_frame", this, "_do_callbacks");
            break;
        }
        case NOTIFICATION_EXIT_TREE:
        {
            _pool->stop();
            delete _pool;

            get_tree()->disconnect("idle_frame", this, "_do_callbacks");
            break;
        }
        default: break;
    }

}

int Pathfinder::compute_path(Vector2 initial_world, Vector2 goal_world, Ref<CharacterParameters> character_parameters, Rect2 region, Array dynamic_masses_world, Object* obj, String method) {
    static pathfinding::Settings empty { 0, 1, 1, 1, false };

    if (!_graph) {
        return -1;
    }

    pathfinding::Graph graph = _graph->graph();
    Ref<Grid> grid = RES();
    if (_graph) grid = _graph->grid_get();

    Vector2 initialv = !grid.is_null() ? grid->gridded(initial_world) : initial_world.floor();
    Vector2 goalv = !grid.is_null() ? grid->gridded(goal_world) : goal_world.floor();

    auto initial = pathfinding::State::create((int)initialv.x, (int)initialv.y);
    auto goal = pathfinding::State::create((int)goalv.x, (int)goalv.y);

    pathfinding::Region rgion;
    rgion.x = region.position.snapped(Vector2(1, 1)).x;
    rgion.y = region.position.snapped(Vector2(1, 1)).y;
    rgion.w = region.size.snapped(Vector2(1, 1)).width;
    rgion.h = region.size.snapped(Vector2(1, 1)).height;

    int id = _id_counter++;
    _id_counter = _id_counter % (1 << 30);

    // Add dynamic masses to the graph
    for (int i = 0; i < dynamic_masses_world.size(); i++) {
        Rect2 rect = dynamic_masses_world[i];
        rect = rect.abs();
        Vector2 top_left = _graph->world_to_graph(rect.position);
        Vector2 size = _graph->graph_units(rect.size).ceil();

        for (real_t x = top_left.x; x < top_left.x + size.x; x++) {
            for (real_t y = top_left.y; y < top_left.y + size.y; y++) {
                if (graph.get_at(x, y) != pathfinding::AIR_TILEKIND) continue;
                graph.set_at(x, y, pathfinding::CHARACTER_TILEKIND);
            }
        }
    }

    _compute_path_async(id, graph, character_parameters.is_null() ? empty : character_parameters->settings(), rgion, initial, goal.x, goal.y, obj, method);

    return id;
}

void Pathfinder::cancel(int id) {
    _callbacks.erase(id);
}

void Pathfinder::_compute_path_async(
    int id,
    const pathfinding::Graph& graph,
    const pathfinding::Settings& settings,
    const pathfinding::Region& region,
    const pathfinding::State& initial,
    const int goal_x, const int goal_y,
    Object* obj, String method) {
    
    if (!_pool) {
        if (!obj || !obj->has_method(method)) return;
        obj->call(method, Dictionary());
        return;
    }

    _callbacks[id] = std::pair<Object*, String>(obj, method);

    _pool->push(
        [this, id, region, graph, settings, initial, goal_x, goal_y]() {
            std::vector<pathfinding::State> path;

            pathfinding::search(graph, settings, region, initial, goal_x, goal_y, path);

            if (_filtered) {
                pathfinding::filter(graph, settings, path);
            }

            PoolVector2Array gd_path;
            PoolIntArray scenarios;
            for (auto& state : path) {
                gd_path.push_back(Vector2(state.x, state.y));
                scenarios.push_back((Scenario)state.scenario_meta);
            }
                
            Dictionary dict;
            dict["path"] = gd_path;
            dict["scenarios"] = scenarios;

            {
                std::unique_lock<std::mutex> lock(_lock);
                this->_results.push_back(std::pair<unsigned int, Dictionary>(id, dict));
            }
        }
    );
}
