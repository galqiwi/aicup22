#include "Strategy.h"
#include "Constants.h"
#include "DebugSingleton.h"
#include "World.h"

#include <cassert>
#include <iostream>

namespace Emulator {

TStrategyAction GenerateRandomAction(int actionDuration) {
    assert(GetGlobalConstants());

    auto speed = RandomUniformVector() * GetGlobalConstants()->maxUnitForwardSpeed * 2;

    if (rand() % 20 == 0) {
        speed = {0, 0};
    }

    return {
        .Speed = speed,
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

    if (currentActionId == -1) {
        currentActionId = static_cast<int>(Actions.size()) - 1;
    }

    auto& action = Actions[currentActionId];

    auto targetDirection = abs(action.Speed) > 0.01 ? norm(action.Speed):world.UnitsById.find(unitId)->second.Direction;
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

TStrategy TStrategy::Mutate() {
    int mutationIndex = (int)(rand() % Actions.size());
    auto output = *this;

    output.Actions[mutationIndex].Speed = output.Actions[mutationIndex].Speed + RandomUniformVector() * GetGlobalConstants()->maxUnitForwardSpeed * 0.2;
    return output;
}

void VisualiseStrategy(const TStrategy& strategy, const TWorld &world, int unitId, int untilTick) {
    TWorld currentWorld = world;
    auto& unit = currentWorld.UnitsById[unitId];

    std::vector<model::Vec2> line;
    line.reserve(untilTick - currentWorld.CurrentTick);

    auto foo = currentWorld.ProjectileById.size();

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();
        line.push_back(unit.Position.ToApi());
        for (auto& [_, projectile]: currentWorld.ProjectileById) {
            GetGlobalDebugInterface()->addCircle(projectile.Position.ToApi(), 0.1, debugging::Color(0, 1, 0, 1));
        }
    }

    assert(GetGlobalDebugInterface());
    GetGlobalDebugInterface()->addPolyLine(std::move(line), 0.15, debugging::Color(1, 0, 0, 1));
}

}
