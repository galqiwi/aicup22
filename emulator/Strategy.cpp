#include "Strategy.h"
#include "Constants.h"
#include "DebugSingleton.h"
#include "World.h"
#include "LootPicker.h"
#include "Evaluation.h"

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

Vector2D GetPreventiveTargetDirection(const TUnit& unit, const TUnit& enemy) {
    auto targetDirection = enemy.Position - unit.Position;

    return norm(targetDirection);

    if (!unit.Weapon) {
        return norm(targetDirection);
    }

    auto projectileSpeed = GetGlobalConstants()->weapons[*unit.Weapon].projectileSpeed;

    auto velocityProjection = enemy.Velocity - targetDirection * ((enemy.Velocity * targetDirection) / abs2(targetDirection));

    auto angleAdjustmentSin = abs(velocityProjection) / projectileSpeed;
    if (fabs(angleAdjustmentSin) < 0.01) {
        return norm(targetDirection);
    }

    auto angleAdjustmentCos = sqrt(1 - angleAdjustmentSin * angleAdjustmentSin);
    return norm(targetDirection) * angleAdjustmentCos + norm(velocityProjection) * angleAdjustmentSin;
}

TOrder TStrategy::GetOrder(const TWorld &world, int unitId) const {
    int currentActionId = -1;
    int actionStartTick = StartTick;
    auto constants = GetGlobalConstants();

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

    const auto& unit = world.UnitById.find(unitId)->second;


    bool isRotation = true;//(world.CurrentTick - world.LastRotationId < constants->ticksPerSecond * 4);
    Vector2D rotationDirection = {unit.Direction.y, -unit.Direction.x};

    // TODO: support multiple players
    if (world.UnitById.size() > 1) {
        std::optional<double> closestDist2;
        int closestUnitId;
        for (const auto& [_, otherUnit]: world.UnitById) {
            if (otherUnit.PlayerId == world.MyId) {
                continue;
            }

            auto dist2 = abs2(unit.Position - otherUnit.Position);
            if (!closestDist2 || dist2 < *closestDist2) {
                closestDist2 = dist2;
                closestUnitId = otherUnit.Id;
            }
        }
        if (closestDist2) {
            if (*closestDist2 > 1600 && isRotation) {
                return {
                    .UnitId = unitId,
                    .TargetVelocity = action.Speed,
                    .TargetDirection = rotationDirection,
                    .Shoot = true,
                    .IsRotation = true,
                };
            }
            return {
                .UnitId = unitId,
                .TargetVelocity = action.Speed,
                .TargetDirection = GetPreventiveTargetDirection(unit, world.UnitById.find(closestUnitId)->second),
                .Shoot = true,
            };
        }
    }

    // pick loot if possible
    auto lootId = GetTargetLoot(world, unitId);
    auto target = GetTarget(world, lootId);
    if (abs(target - unit.Position) < constants->unitRadius) {
        return {
            .UnitId = unitId,
            .TargetVelocity = Vector2D{0, 0},
            .TargetDirection = isRotation ? rotationDirection:unit.Direction,
            .Pickup = lootId.has_value(),
            .LootId = (lootId.has_value() ? *lootId:0),
            .IsRotation = isRotation,
        };
    }


    auto targetDirection = abs(action.Speed) > 0.01 ? norm(action.Speed):unit.Direction;

    return {
        .UnitId = unitId,
        .TargetVelocity = action.Speed,
        .TargetDirection = isRotation ? rotationDirection: targetDirection,
        .UseShieldPotion = (unit.ShieldPotions > 0),
        .IsRotation = isRotation,
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
    auto& unit = currentWorld.UnitById[unitId];

    std::vector<model::Vec2> line;
    line.reserve(untilTick - currentWorld.CurrentTick);

    auto foo = currentWorld.ProjectileById.size();

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.PrepareEmulation();
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();
        line.push_back(unit.Position.ToApi());

        auto score = EvaluateWorld(world, unit);
        debugging::Color color = debugging::Color(0, 1, 0, 1);
        if (!get<1>(score).value) {
            color = debugging::Color(0.5, 0.5, 0.5, 1);
        } else {
            if (*get<1>(score).value > 0) {
                color = debugging::Color(1, 0, 0, 1);
            }
        }
        GetGlobalDebugInterface()->addCircle(unit.Position.ToApi(), 0.1, color);

        for (auto& [_, projectile]: currentWorld.ProjectileById) {
            GetGlobalDebugInterface()->addCircle(projectile.Position.ToApi(), 0.1, debugging::Color(0, 1, 0, 1));
        }
    }

    assert(GetGlobalDebugInterface());
//    GetGlobalDebugInterface()->addPolyLine(std::move(line), 0.15, debugging::Color(1, 0, 0, 1));
}

}
