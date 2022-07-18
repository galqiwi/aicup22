#pragma once

#include "Strategy.h"
#include "World.h"

namespace Emulator {

struct TOptionalDouble {
    std::optional<double> value;
};
bool operator<(TOptionalDouble a, TOptionalDouble b);
TOptionalDouble operator+(TOptionalDouble a, TOptionalDouble b);

using THealthScore = double;
using TTargetDistanceScore = double;
using TCombatSafetyScore = TOptionalDouble;

using TScore = std::tuple<THealthScore, TOptionalDouble, TTargetDistanceScore>;
TScore operator+(TScore a, TScore b);

double GetCombatSafety(const TWorld& world, const TUnit& unit);
double GetCombatSafety(const TWorld& world, const TUnit& unit, Vector2D unitPosition);
TScore EvaluateWorld(const TWorld& world, const TUnit& unit);
TScore EvaluateStrategy(const TStrategy& strategy, const TWorld& world, int unitId, int untilTick);

}
