#pragma once

#include "public.h"
#include "Vector2D.h"

#include <vector>

namespace Emulator {

struct TStrategyAction {
    Vector2D Speed;
    int ActionDuration;
};

struct TStrategy {
    int StartTick;

    std::vector<TStrategyAction> Actions;

    [[nodiscard]] TOrder GetOrder(const TWorld& world, int unitId) const;
};

TStrategy GenerateRandomStrategy(int startTick, int actionDuration, int nActions);

}
