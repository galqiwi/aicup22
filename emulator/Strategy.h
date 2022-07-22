#pragma once

#include "public.h"
#include "Vector2D.h"

#include <vector>
#include <optional>

namespace Emulator {

struct TStrategyAction {
    Vector2D Speed;
    int ActionDuration;
};

enum EObedienceLevel {
    DEFAULT = 0,
    VERY_SOFT = 1,
    SOFT = 2,
    HARD = 3,
};

struct TStrategy {
    int StartTick;

    std::vector<TStrategyAction> Actions;

    std::optional<Vector2D> GoTo;

    [[nodiscard]] TOrder GetOrder(const TWorld& world, int unitId, bool forSimulation = true) const;
    TStrategy Mutate();

    TStrategyAction GetAction(const TWorld& world, int unitId, int tickId) const;
    TOrder GetResGatheringOrder(const TWorld& world, int unitId, bool forSimulation = true) const;

    EObedienceLevel ObedienceLevel{DEFAULT};
};

TStrategy GenerateRandomStrategy(int startTick, int actionDuration, int nActions);

TStrategy GenerateRunaway(Vector2D direction);

void VisualiseStrategy(const TStrategy& strategy, const TWorld &world, int unitId, int untilTick);

}
