#include "register_types.h"

#include "core/class_db.h"
#include "grid.hpp"
#include "gridded_graph.hpp"
#include "pathfinder.hpp"
#include "character_parameters.hpp"

void register_pathfinder_types() {
    ClassDB::register_class<Grid>();
    ClassDB::register_class<GriddedGraph>();
    ClassDB::register_class<Pathfinder>();
    ClassDB::register_class<CharacterParameters>();
}

void unregister_pathfinder_types() {
}
