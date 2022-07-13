#pragma once

#include "Strategy.h"
#include "World.h"

namespace Emulator {

using THealthScore = double;
using TTargetDistanceScore = double;
struct TCombatSafetyScore {
    std::optional<double> value;
};

bool operator<(TCombatSafetyScore a, TCombatSafetyScore b);
TCombatSafetyScore operator+(TCombatSafetyScore a, TCombatSafetyScore b);

using TScore = std::tuple<THealthScore, TCombatSafetyScore, TTargetDistanceScore>;
TScore operator+(TScore a, TScore b);

TScore EvaluateWorld(const TWorld& world, const TUnit& unit);
TScore EvaluateStrategy(const TStrategy& strategy, const TWorld& world, int unitId, int untilTick);

}
