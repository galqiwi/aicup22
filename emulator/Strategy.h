#pragma once

#include "public.h"
#include "Vector2D.h"

#include <vector>

namespace Emulator {

struct TStrategyAction {
    Vector2D Speed;
    int ActionDuration;
};

enum EObedienceLevel {
    DEFAULT = 0,
    SOFT = 1,
    HARD = 2,
};

struct TStrategy {
    int StartTick;

    std::vector<TStrategyAction> Actions;

    [[nodiscard]] TOrder GetOrder(const TWorld& world, int unitId, bool forSimulation = true) const;
    TStrategy Mutate();

    const TStrategyAction& GetAction(int tickId) const;
    TOrder GetResGatheringOrder(const TWorld& world, int unitId, bool forSimulation = true) const;

    EObedienceLevel ObedienceLevel{DEFAULT};
};

TStrategy GenerateRandomStrategy(int startTick, int actionDuration, int nActions);

TStrategy GenerateRunaway(Vector2D direction);

void VisualiseStrategy(const TStrategy& strategy, const TWorld &world, int unitId, int untilTick);

}
