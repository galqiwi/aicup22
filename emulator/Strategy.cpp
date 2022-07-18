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

//    return norm(targetDirection);

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

TOrder TStrategy::GetOrder(const TWorld &world, int unitId, bool forSimulation) const {
    int currentActionId = -1;
    int actionStartTick = StartTick;
    auto constants = GetGlobalConstants();
    assert(constants);

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

    assert(world.StateByUnitId.find(unitId) != world.StateByUnitId.end());
    const auto& unitState = world.StateByUnitId.find(unitId)->second;

    bool isRotationStart = (world.CurrentTick - unitState.LastRotationTick >= constants->ticksPerSecond * 2);
    bool isRotation = isRotationStart || (world.CurrentTick - unitState.LastRotationTick < constants->ticksPerSecond * 1);
    Vector2D rotationDirection = {unit.Direction.y, -unit.Direction.x};

    auto lootId = GetTargetLoot(world, unitId);
    auto target = GetTarget(unitId, world, lootId);
    auto canPick = lootId && (abs(target - unit.Position) < constants->unitRadius);

    // TODO: support multiple players
    if (world.UnitById.size() > 1 && unit.Weapon == 2 && unit.Ammo[2] > 0) {
        std::optional<double> closestDist2;
        int closestUnitId;
        for (const auto& [_, otherUnit]: world.UnitById) {
            if (otherUnit.Imaginable) {
                continue;
            }
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
            const auto& otherUnit = world.UnitById.find(closestUnitId)->second;
            auto actionRadius = std::max(otherUnit.GetCombatRadius(), unit.GetCombatRadius());

            if (*closestDist2 < actionRadius * actionRadius) {
                bool shoot = true;
                if (!forSimulation) {
                    shoot = !constants->obstaclesMeta.SegmentIntersectsObstacle(unit.Position, otherUnit.Position);
                }

                if (otherUnit.RemainingSpawnTime > 0) {
                    shoot = false;
                }

                return {
                    .UnitId = unitId,
                    .TargetVelocity = action.Speed,
                    .TargetDirection = GetPreventiveTargetDirection(unit, world.UnitById.find(closestUnitId)->second),
                    .Aim = true,
                    .Shoot = shoot,
                    .Pickup = canPick,
                    .LootId = lootId.value_or(0),
                };
            }
        }
    }

    if (canPick) {
        return {
            .UnitId = unitId,
            .TargetVelocity = Vector2D{0, 0},
            .TargetDirection = isRotation ? rotationDirection:unit.Direction,
            .Pickup = lootId.has_value(),
            .LootId = lootId.value_or(0),
            .IsRotationStart = isRotationStart,
        };
    }


    auto targetDirection = abs(action.Speed) > 0.01 ? norm(action.Speed):unit.Direction;

    return {
        .UnitId = unitId,
        .TargetVelocity = action.Speed,
        .TargetDirection = isRotation ? rotationDirection: targetDirection,
        .UseShieldPotion = (unit.ShieldPotions > 0),
        .IsRotationStart = isRotationStart,
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

    while (currentWorld.CurrentTick < untilTick) {
        currentWorld.PrepareEmulation();
        currentWorld.EmulateOrder(strategy.GetOrder(currentWorld, unitId));
        currentWorld.Tick();
        line.push_back(unit.Position.ToApi());

        auto score = EvaluateWorld(world, unit);
        debugging::Color color = debugging::Color(0, 1, 0, 1);
        if (score.CombatSafetyScore.value) {
            color = debugging::Color(0.5, 0.5, 0.5, 1);
        } else {
            if (score.CombatSafetyScore.value > 0) {
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

TStrategy GenerateRunaway(Vector2D direction) {
    auto constants = GetGlobalConstants();
    assert(constants);

    return {
        .StartTick = 0,
        .Actions = {TStrategyAction{
            .Speed = norm(direction) * constants->maxUnitForwardSpeed,
            .ActionDuration = 1,
        }},
    };
}

}
