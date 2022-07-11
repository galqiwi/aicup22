#include "Strategy.h"
#include "Constants.h"
#include "World.h"

#include <cassert>

namespace Emulator {

TStrategyAction GenerateRandomAction(int actionDuration) {
    assert(GetGlobalConstants());

    return {
        .Speed = RandomUniformVector() * GetGlobalConstants()->maxUnitForwardSpeed * 2,
        .ActionDuration = actionDuration,
    };
}

TStrategy GenerateRandomStrategy(int startTick, int actionDuration, int nActions) {
    std::vector<TStrategyAction> actions;
    actions.reserve(nActions);

    for (int i = 0; i < nActions; ++i) {
        actions.push_back(GenerateRandomAction(actionDuration));
    }

    return {
        .StartTick = startTick,
        .Actions = std::move(actions),
    };
}

TOrder TStrategy::GetOrder(const TWorld &world, int unitId) const {
    int currentActionId = -1;
    int actionStartTick = StartTick;

    for (int i = 0; i < Actions.size(); ++i) {
        auto& action = Actions[i];

        if (actionStartTick > world.CurrentTick) {
            break;
        }
        if (actionStartTick <= world.CurrentTick && world.CurrentTick < actionStartTick + action.ActionDuration) {
            currentActionId = i;
            break;
        }

        actionStartTick += action.ActionDuration;
    }

    assert(currentActionId != -1);
    auto& action = Actions[currentActionId];

    auto targetDirection = norm(action.Speed);
    bool shoot = false;

    // TODO: support multiple players
    if (world.UnitsById.size() > 1) {
        for (const auto& [_, unit]: world.UnitsById) {
            if (unit.PlayerId == world.MyId) {
                continue;
            }
            targetDirection = norm(unit.Position - world.UnitsById.find(unitId)->second.Position);
            shoot = true;
            break;
        }
    }

    return {
        .UnitId = unitId,
        .TargetVelocity = action.Speed,
        .TargetDirection = targetDirection,
        .Shoot = shoot,
    };
}

}
