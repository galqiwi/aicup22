#include "World.h"
#include "Strategy.h"

#include "model/Item.hpp"

#include "emulator/LootPicker.h"

#include <cassert>
#include <fstream>
#include <iostream>

namespace Emulator {

model::UnitOrder TOrder::ToApi() const {
    if (Aim || Shoot) {
        return {TargetVelocity.ToApi(), TargetDirection.ToApi(), std::make_shared<model::ActionOrder::Aim>(Shoot)};
    }

    if (Pickup) {
        return {TargetVelocity.ToApi(), TargetDirection.ToApi(), std::make_shared<model::ActionOrder::Pickup>(LootId)};
    }

    if (UseShieldPotion) {
        return {TargetVelocity.ToApi(), TargetDirection.ToApi(), std::make_shared<model::ActionOrder::UseShieldPotion>()};
    }

    return {TargetVelocity.ToApi(), TargetDirection.ToApi(), std::nullopt};
}

void TState::Update(const TWorld& world, const TOrder& order) {
    auto constants = GetGlobalConstants();
    assert(constants);

    if (order.IsRotationStart) {
        LastRotationTick = world.CurrentTick;
    }
    spiralAngle += (constants->maxUnitForwardSpeed / constants->realTicksPerSecond) / (0.75 * world.Zone.nextRadius);
}

EAutomatonState updateAutomatonState(EAutomatonState state, const TWorld& world, int UnitId) {
    auto constants = GetGlobalConstants();
    assert(constants);

    const auto& unit = world.UnitById.find(UnitId)->second;

    if (unit.RemainingSpawnTime && unit.RemainingSpawnTime > 0) {
        return RES_GATHERING;
    }

    if (state == RES_GATHERING) {
        if (unit.Weapon == 2 && unit.Ammo[2] > 0 && unit.Shield > 0) {
            return FIGHT;
        } else {
            return RES_GATHERING;
        }
    }

    if (state == FIGHT) {
        if (unit.Weapon == 2 && unit.Ammo[2] > 0 && unit.Shield > 0) {
            return FIGHT;
        } else {
            return RES_GATHERING;
        }
    }

    abort();
}

void TState::Sync(const TWorld& world) {
    AutomatonState = updateAutomatonState(AutomatonState, world, UnitId);
}

TWorld TWorld::FormApi(const model::Game& game) {
    auto constants = GetGlobalConstants();
    assert(constants);

    TWorld output;

    for (const auto& unit: game.units) {
        output.UnitById[unit.id] = TUnit{
            .Id = unit.id,
            .PlayerId = unit.playerId,
            .Position = Vector2D::FromApi(unit.position),
            .Direction = Vector2D::FromApi(unit.direction),
            .Velocity = Vector2D::FromApi(unit.velocity),
            .Health = unit.health,
            .Shield = unit.shield,
            .ExtraLives = unit.extraLives,
            .RemainingSpawnTime = unit.remainingSpawnTime,
            .Aim = unit.aim,
            .HealthRegenerationStartTick = unit.healthRegenerationStartTick,
            .Weapon = unit.weapon,
            .NextShotTick = unit.nextShotTick,
            .Ammo = unit.ammo,
            .ShieldPotions = unit.shieldPotions,
        };
    }
    output.CurrentTick = game.currentTick;
    output.MyId = game.myId;

    if (!output.Constants_) {
        output.Constants_ = GetGlobalConstants();
    }

    output.Zone = {
        .currentCenter = Vector2D::FromApi(game.zone.currentCenter),
        .currentRadius = game.zone.currentRadius,
        .nextCenter = Vector2D::FromApi(game.zone.nextCenter),
        .nextRadius = game.zone.nextRadius,
    };

    output.ProjectileById.reserve(game.projectiles.size());
    for (auto& projectile: game.projectiles) {
        output.ProjectileById.insert({projectile.id, {
            .Id = projectile.id,
            .WeaponTypeIndex = projectile.weaponTypeIndex,
            .ShooterId = projectile.shooterId,
            .ShooterPlayerId = projectile.shooterPlayerId,
            .Position = Vector2D::FromApi(projectile.position),
            .Velocity = Vector2D::FromApi(projectile.velocity),
            .LifeTime = projectile.lifeTime,
        }});
    }

    output.LootById.reserve(game.loot.size());
    for (auto& loot: game.loot) {
        TLoot newLoot = {
            .Id = loot.id,
            .Position = Vector2D::FromApi(loot.position),
        };

        if (auto weapon = dynamic_cast<model::Item::Weapon*>(loot.item.get())) {
            if (weapon->typeIndex != 2) {
                continue;
            }
            newLoot.Item = Weapon;
            newLoot.WeaponType = weapon->typeIndex;
        }
        if (auto ammo = dynamic_cast<model::Item::Ammo*>(loot.item.get())) {
            if (ammo->weaponTypeIndex != 2) {
                continue;
            }
            newLoot.Item = Ammo;
            newLoot.WeaponType = ammo->weaponTypeIndex;
            newLoot.Amount = ammo->amount;
        }
        if (auto potions = dynamic_cast<model::Item::ShieldPotions*>(loot.item.get())) {
            newLoot.Item = ShieldPotions;
            newLoot.Amount = potions->amount;
        }

        output.LootById.insert({newLoot.Id, newLoot});
    }

    for (const auto& [_, unit]: output.UnitById) {
        if (unit.PlayerId != output.MyId) {
            continue;
        }

        auto& preprocessedData = output.PreprocessedDataById[unit.Id];

        for (const auto& [projectileId, projectile]: output.ProjectileById) {
            if (SegmentIntersectsCircle(projectile.Position, projectile.Position + projectile.Velocity * projectile.LifeTime, unit.Position, constants->unitRadius)) {
                preprocessedData.InDanger = true;
            }
        }

        for (const auto& [otherUnitId, otherUnit]: output.UnitById) {
            if (otherUnit.PlayerId != unit.PlayerId) {
                continue;
            }
            preprocessedData.Friends.push_back(otherUnitId);
        }
    }


    return output;
}

void TWorld::Emulate(const std::vector<TOrder> &orders) {
    if (!Constants_) {
        Constants_ = GetGlobalConstants();
    }

    for (const auto &order: orders) {
        EmulateOrder(order);
    }
}

void TWorld::EmulateOrder(const TOrder &order) {
    assert(Constants_);
    assert(Constants_->maxUnitBackwardSpeed < Constants_->maxUnitForwardSpeed);

    auto unitId = order.UnitId;

    assert(UnitById.contains(unitId));

    auto &unit = UnitById[unitId];

    auto targetVelocity = ClipVelocity(order.TargetVelocity, unit);
    auto velocity = ApplyAcceleration(unit.Velocity, targetVelocity);
    MoveCollidingUnit(unit, velocity);

    RotateUnit(unit, order.TargetDirection);
}

void TWorld::RotateUnit(TUnit &unit, Vector2D targetDirection) {
    if (abs2(targetDirection) <= Constants_->unitRadius * Constants_->unitRadius / 4) {
        return;
    }

    targetDirection = norm(targetDirection);
    auto normal_vector = targetDirection - unit.Direction * ((targetDirection * unit.Direction) / abs2(unit.Direction));
    auto direction = norm(unit.Direction);

    double maxAngle = Constants_->rotationSpeed / 180 * M_PI / Constants_->ticksPerSecond;

    if (normal_vector * targetDirection < sin(maxAngle)) {
        unit.Direction = targetDirection;
    } else {
        unit.Direction = direction * cos(maxAngle) + normal_vector * sin(maxAngle);
    }
}

Vector2D TWorld::ClipVelocity(Vector2D velocity, const TUnit &unit) {
    if (abs2(velocity) < Constants_->maxUnitBackwardSpeed * Constants_->maxUnitBackwardSpeed) {
        return velocity;
    }

    auto projection = (norm(unit.Direction) * norm(velocity)) * (Constants_->maxUnitForwardSpeed - Constants_->maxUnitBackwardSpeed) / 2;

    auto limit = sqrt(projection * projection + Constants_->maxUnitBackwardSpeed * Constants_->maxUnitForwardSpeed) + projection;

    if (unit.Weapon) {
        // TODO: better aim simulation
        limit *= (1 - (1 - Constants_->weapons[*unit.Weapon].aimMovementSpeedModifier) * unit.Aim);
    }

    if (abs(velocity) < limit) {
        return velocity;
    }

    return norm(velocity) * limit;
}

Vector2D TWorld::ApplyAcceleration(Vector2D velocity, Vector2D targetVelocity) {
    auto desiredDelta = targetVelocity - velocity;

    auto maxDeltaChange = Constants_->unitAcceleration / Constants_->ticksPerSecond;

    if (abs(desiredDelta) > maxDeltaChange) {
        velocity = velocity + norm(desiredDelta) * maxDeltaChange;
    } else {
        velocity = targetVelocity;
    }

    return velocity;
}

void TWorld::MoveCollidingUnit(TUnit& unit, Vector2D velocity) {
    assert(Constants_);

    unit.Velocity = velocity;

    if (!Constants_->obstaclesMeta.IsInitialized()) {
        Constants_->obstaclesMeta = TObstacleMeta(Constants_->obstacles);
    }

    for (const auto& obstacleId: Constants_->obstaclesMeta.GetIntersectingIds(unit.Position)) {
        if (unit.RemainingSpawnTime > 0) {
            break;
        }
        auto& obstacle = Constants_->obstacles[obstacleId];

        if (abs2(obstacle.Center - unit.Position) > (obstacle.Radius + Constants_->unitRadius) * (obstacle.Radius + Constants_->unitRadius)) {
            continue;
        }

        auto normalVector = obstacle.Center - unit.Position;

        unit.Position = obstacle.Center + norm(unit.Position - obstacle.Center) * (obstacle.Radius + Constants_->unitRadius);

        if (normalVector * velocity < 0) {
            continue;
        }

        unit.Velocity = unit.Velocity - normalVector * ((normalVector * unit.Velocity) / abs2(normalVector));
    }

    unit.Position = unit.Position + unit.Velocity / Constants_->ticksPerSecond;
}

void TWorld::PrepareEmulation() {
    if (!Constants_) {
        Constants_ = GetGlobalConstants();
    }
    assert(Constants_);

    std::vector<int> idsToErase;

    for (auto& [_, projectile]: ProjectileById) {
        projectile.LifeTime -= 1 / Constants_->ticksPerSecond;
        if (projectile.LifeTime < 0) {
            idsToErase.push_back(projectile.Id);
            continue;
        }
        auto newPosition = projectile.Position + projectile.Velocity / Constants_->ticksPerSecond;

        auto obstacle = Constants_->obstaclesMeta.GetObstacle(projectile.Position);
        if (obstacle && !Constants_->obstacles[*obstacle].CanShootThrough) {
            idsToErase.push_back(projectile.Id);
            continue;
        }

        int shooterPlayerId = -1;
        if (UnitById.contains(projectile.ShooterId)) {
            shooterPlayerId = UnitById.find(projectile.ShooterId)->second.PlayerId;
        }

        for (auto& [unitId, unit]: UnitById) {
            // TODO: microticks or other stuff
            if (SegmentIntersectsCircle(projectile.Position, projectile.Position + (projectile.Velocity - unit.Velocity) / Constants_->ticksPerSecond, unit.Position, Constants_->unitRadius)) {
                if (unit.PlayerId != shooterPlayerId) {
                    unit.Health -= Constants_->weapons[projectile.WeaponTypeIndex].projectileDamage;
                }
                idsToErase.push_back(projectile.Id);
                break;
            }
        }

        projectile.Position = newPosition;
    }

    for (auto& [unitId, unit]: UnitById) {
        if (unit.PlayerId == MyId) {
            continue;
        }

        unit.Position = unit.Position + unit.Velocity / Constants_->ticksPerSecond;
    }

    for (auto id: idsToErase) {
        ProjectileById.erase(id);
    }

    for (auto& [unitId, unit]: UnitById) {
        if (unit.PlayerId != MyId) {
            continue;
        }
        if (abs(unit.Position - Zone.currentCenter) > Zone.currentRadius - Constants_->unitRadius * 3) {
            unit.Health -= Constants_->zoneDamagePerSecond / Constants_->ticksPerSecond;
        }
    }
    for (auto& [unitId, unit]: UnitById) {
        if (unit.PlayerId != MyId) {
            continue;
        }
        if (!unit.RemainingSpawnTime) {
            continue;
        }
        unit.RemainingSpawnTime = *unit.RemainingSpawnTime - 1 / Constants_->ticksPerSecond;
        if (unit.RemainingSpawnTime <= 0) {
            unit.RemainingSpawnTime = std::nullopt;
        }

        for (auto obstacleId: Constants_->obstaclesMeta.GetIntersectingIds(unit.Position)) {
            const auto& obstacle = Constants_->obstacles[obstacleId];
            if (abs(obstacle.Center - unit.Position) <= (obstacle.Radius + Constants_->unitRadius - 0.08)) {
                // it's hard to make a proper simulation, so let's just assume that this damage is bad
                unit.Health -= 10000;
            }
        }
    }
}

void TWorld::Tick() {
    assert(Constants_);

    ++CurrentTick;
    Zone.currentRadius -= Constants_->zoneSpeed / Constants_->ticksPerSecond;
}

const std::string LOAD_DUMP_VERSION = "6.0";

void TWorld::Dump(const char *filename) {
    std::ofstream fout(filename);
    fout.precision(20);

    fout << LOAD_DUMP_VERSION << std::endl;

    fout << *GetGlobalConstants() << std::endl;

    fout << CurrentTick << std::endl;

    fout << MyId << std::endl;

    fout << UnitById.size() << std::endl;
    for (const auto& [_, unit]: UnitById) {
        fout << unit.Id << std::endl;
        fout << unit.PlayerId << std::endl;
        fout << unit.Position << std::endl;
        fout << unit.Direction << std::endl;
        fout << unit.Velocity << std::endl;
    }
}

void TWorld::Load(const char *filename) {
    std::ifstream fin(filename);
    fin.precision(20);
    UnitById = {};

    std::string version;
    fin >> version;
    assert(version == LOAD_DUMP_VERSION);

    TConstants constants;
    fin >> constants;

    if (!GetGlobalConstants()) {
        SetGlobalConstants(std::move(constants));
    }

    Constants_ = GetGlobalConstants();

    fin >> CurrentTick;

    fin >> MyId;

    int unitsByIdSize;
    fin >> unitsByIdSize;

    for (int i = 0; i < unitsByIdSize; ++i) {
        TUnit unit{};
        fin >> unit.Id;
        fin >> unit.PlayerId;
        fin >> unit.Position;
        fin >> unit.Direction;
        fin >> unit.Velocity;
        UnitById[unit.PlayerId] = unit;
    }
}
void TWorld::UpdateLootIndex() {
    LootByItemIndex.clear();
    for (auto& [_, loot]: LootById) {
        LootByItemIndex[loot.Item].push_back(loot);
    }
    // initialisation
    LootByItemIndex[Weapon];
    LootByItemIndex[ShieldPotions];
    LootByItemIndex[Ammo];
}

void TWorld::UpdateUnitsTargetLoot() {
    LootIdByUnitId = std::nullopt;

    std::unordered_map<int, std::optional<int>> lootIdByUnitId;
    for (auto& [unitId, unit]: UnitById) {
        if (unit.PlayerId != MyId) {
            continue;
        }
        lootIdByUnitId[unitId] = GetTargetLoot(*this, unitId);
    }

    LootIdByUnitId = std::move(lootIdByUnitId);
}

double TUnit::GetCombatRadius() const {
    static auto constants = GetGlobalConstants();
    assert(constants);

    if (!Weapon) {
        return 0;
    }
    const auto& weaponProperties = constants->weapons[*Weapon];
    return weaponProperties.projectileSpeed * weaponProperties.projectileLifeTime;
}

}
