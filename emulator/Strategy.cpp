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

const TStrategyAction& TStrategy::GetAction(int tickId) const {
    int currentActionId = -1;
    int actionStartTick = StartTick;

    for (int i = 0; i < Actions.size(); ++i) {
        auto& action = Actions[i];

        if (actionStartTick > tickId) {
            break;
        }
        if (actionStartTick <= tickId && tickId < actionStartTick + action.ActionDuration) {
            currentActionId = i;
            break;
        }

        actionStartTick += action.ActionDuration;
    }

    if (currentActionId == -1) {
        currentActionId = static_cast<int>(Actions.size()) - 1;
    }

    return Actions[currentActionId];
}

TOrder TStrategy::GetResGatheringOrder(const TWorld &world, int unitId, bool forSimulation) const {
    const auto& state = world.StateByUnitId.find(unitId)->second;
    auto constants = GetGlobalConstants();
    assert(constants);
    auto action = GetAction(world.CurrentTick);
    const auto& unit = world.UnitById.find(unitId)->second;
    assert(world.StateByUnitId.find(unitId) != world.StateByUnitId.end());
    const auto& unitState = world.StateByUnitId.find(unitId)->second;
    bool isRotationStart = (world.CurrentTick - state.LastRotationTick >= constants->ticksPerSecond * 2);
    bool isRotation = isRotationStart || (world.CurrentTick - state.LastRotationTick < constants->ticksPerSecond * 1);
    Vector2D rotationDirection = {unit.Direction.y, -unit.Direction.x};
    auto lootId = GetTargetLoot(world, unitId, /* forSimulation */ true);
    auto target = GetTarget(unitId, world, lootId);
    auto canPick = lootId && (abs(target - unit.Position) < constants->unitRadius);

    if (!forSimulation && !canPick) {
        lootId = GetTargetLoot(world, unitId, false);
        target = GetTarget(unitId, world, lootId);
        canPick = lootId && (abs(target - unit.Position) < constants->unitRadius);
    }

    auto targetDirection = abs(action.Speed) > 0.01 ? norm(action.Speed):unit.Direction;

    if (canPick) {
        return {
            .UnitId = unitId,
            .TargetVelocity = action.Speed,
            .TargetDirection = isRotation ? rotationDirection:targetDirection,
            .Pickup = lootId.has_value(),
            .LootId = lootId.value_or(0),
            .IsRotationStart = isRotationStart,
        };
    }

    return {
        .UnitId = unitId,
        .TargetVelocity = action.Speed,
        .TargetDirection = isRotation ? rotationDirection: targetDirection,
        .UseShieldPotion = (unit.ShieldPotions > 0),
        .IsRotationStart = isRotationStart,
    };
}

TOrder TStrategy::GetOrder(const TWorld &world, int unitId, bool forSimulation) const {
    const auto& state = world.StateByUnitId.find(unitId)->second;
    const auto& preprocessedData = world.PreprocessedDataById.find(unitId)->second;

    if (ObedienceLevel == HARD) {
        auto action = GetAction(world.CurrentTick);
        const auto& unit = world.UnitById.find(unitId)->second;

        return {
            .UnitId = unitId,
            .TargetVelocity = action.Speed,
            .TargetDirection = abs(action.Speed) > 0.01 ? norm(action.Speed):unit.Direction,
        };
    }

    if (state.AutomatonState == RES_GATHERING) {
        return GetResGatheringOrder(world, unitId, forSimulation);
    }

    auto constants = GetGlobalConstants();
    assert(constants);

    auto action = GetAction(world.CurrentTick);

    const auto& unit = world.UnitById.find(unitId)->second;

    assert(world.StateByUnitId.find(unitId) != world.StateByUnitId.end());
    const auto& unitState = world.StateByUnitId.find(unitId)->second;

    bool isRotationStart = (world.CurrentTick - state.LastRotationTick >= constants->ticksPerSecond * 2);
    bool isRotation = isRotationStart || (world.CurrentTick - state.LastRotationTick < constants->ticksPerSecond * 1);
    Vector2D rotationDirection = {unit.Direction.y, -unit.Direction.x};

    auto lootId = GetTargetLoot(world, unitId, /* forSimulation */ true);
    auto target = GetTarget(unitId, world, lootId);
    auto canPick = lootId && (abs(target - unit.Position) < constants->unitRadius);

    if (!forSimulation && !canPick) {
        lootId = GetTargetLoot(world, unitId, false);
        target = GetTarget(unitId, world, lootId);
        canPick = lootId && (abs(target - unit.Position) < constants->unitRadius);
    }

    {
        std::optional<double> closestDist2;
        int closestUnitId;
        for (const auto& [_, otherUnit]: world.UnitById) {
            if (otherUnit.Imaginable && abs(otherUnit.Position - unit.Position) > constants->viewDistance) {
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

            if (*closestDist2 < actionRadius * actionRadius && unit.Weapon) {
                bool shoot = world.CurrentTick >= unit.NextShotTick;

                int64_t reloadTicks = lround(ceil(constants->ticksPerSecond / constants->weapons[*unit.Weapon].roundsPerSecond));
                int64_t aimWindow = reloadTicks / 2 + reloadTicks % 2;
                bool aim = (world.CurrentTick >= unit.NextShotTick - aimWindow);

                if (shoot && !forSimulation) {
                    shoot = !constants->obstaclesMeta.SegmentIntersectsObstacle(unit.Position, otherUnit.Position);
                }

                if (otherUnit.RemainingSpawnTime > 0) {
                    shoot = false;
                }

                for (auto friendId: preprocessedData.Friends) {
                    if (friendId == unit.Id) {
                        continue;
                    }

                    assert(world.UnitById.contains(friendId));
                    const auto& friendUnit = world.UnitById.find(friendId)->second;
                    if (SegmentIntersectsCircle(unit.Position, otherUnit.Position, friendUnit.Position, constants->unitRadius)) {
                        shoot = false;
                        break;
                    }
                }

                auto direction = GetPreventiveTargetDirection(unit, world.UnitById.find(closestUnitId)->second);

                if (ObedienceLevel == SOFT) {
                    auto fov = constants->fieldOfView;
                    if (unit.Weapon) {
                        fov -= unit.Aim * (constants->fieldOfView - constants->weapons[*unit.Weapon].aimFieldOfView);
                    }

                    direction = CropDirection(abs(action.Speed) > 0.01 ? norm(action.Speed):direction, world.UnitById.find(closestUnitId)->second.Position - unit.Position, fov / 180 * M_PI / 2 / 3);
                }

                return {
                    .UnitId = unitId,
                    .TargetVelocity = action.Speed,
                    .TargetDirection = direction,
                    .Aim = aim,
                    .Shoot = shoot,
                };
            }
        }
    }

    auto targetDirection = abs(action.Speed) > 0.01 ? norm(action.Speed):unit.Direction;

    if (ObedienceLevel != DEFAULT) {
        isRotation = false;
    }

    if (canPick) {
        return {
            .UnitId = unitId,
            .TargetVelocity = action.Speed,
            .TargetDirection = isRotation ? rotationDirection:targetDirection,
            .Pickup = lootId.has_value(),
            .LootId = lootId.value_or(0),
            .IsRotationStart = isRotationStart,
        };
    }

    return {
        .UnitId = unitId,
        .TargetVelocity = action.Speed,
        .TargetDirection = isRotation ? rotationDirection:targetDirection,
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
