#pragma once

#include "Strategy.h"
#include "World.h"

namespace Emulator {

double EvaluateStrategy(const TStrategy& strategy, const TWorld& world, int unitId, int untilTick);

}
