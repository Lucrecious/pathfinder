#pragma once

namespace pathfinding {
    
enum Scenario {
    Scenario_None = 0x0,
    Scenario_OnFloor = 0x1,
    Scenario_InAir = 0x2,
    Scenario_OnLedgeLeft = 0x4,
    Scenario_OnLedgeRight = 0x8,
};
    
}
