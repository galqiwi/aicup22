#include "World.h"
#include "Strategy.h"

#include <cassert>
#include <fstream>
#include <iostream>

namespace Emulator {

model::UnitOrder TOrder::ToApi() const {
    if (Shoot) {
        return {TargetVelocity.ToApi(), TargetDirection.ToApi(), std::make_shared<model::ActionOrder::Aim>(true)};
    } else {
        return {TargetVelocity.ToApi(), TargetDirection.ToApi(), std::nullopt};
    }
}

TWorld TWorld::FormApi(const model::Game& game) {
    TWorld output;
    for (const auto& unit: game.units) {
        output.UnitsById[unit.id] = TUnit{
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

    assert(UnitsById.contains(unitId));

    auto &unit = UnitsById[unitId];

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

void TWorld::Tick() {
    if (!Constants_) {
        Constants_ = GetGlobalConstants();
    }
    assert(Constants_);

    ++CurrentTick;

    std::vector<int> idsToErase;

    for (auto& [_, projectile]: ProjectileById) {
        projectile.LifeTime -= 1 / Constants_->ticksPerSecond;
        if (projectile.LifeTime < 0) {
            idsToErase.push_back(projectile.Id);
            continue;
        }
        projectile.Position = projectile.Position + projectile.Velocity / Constants_->ticksPerSecond;

        auto obstacle = Constants_->obstaclesMeta.GetObstacle(projectile.Position);
        if (obstacle && !Constants_->obstacles[*obstacle].CanShootThrough) {
            idsToErase.push_back(projectile.Id);
            continue;
        }

        for (auto& [unitId, unit]: UnitsById) {
            // TODO: microticks or other stuff
            if (abs2(projectile.Position - unit.Position) < Constants_->unitRadius * Constants_->unitRadius) {
                unit.Health -= Constants_->weapons[projectile.WeaponTypeIndex].projectileDamage;
                idsToErase.push_back(projectile.Id);
                break;
            }
        }
    }

    for (auto& [unitId, unit]: UnitsById) {
        if (unit.PlayerId == MyId) {
            continue;
        }

        unit.Position = unit.Position + unit.Velocity / Constants_->ticksPerSecond;
    }

    for (auto id: idsToErase) {
        ProjectileById.erase(id);
    }
}

const std::string LOAD_DUMP_VERSION = "6.0";

void TWorld::Dump(const char *filename) {
    std::ofstream fout(filename);
    fout.precision(20);

    fout << LOAD_DUMP_VERSION << std::endl;

    fout << *GetGlobalConstants() << std::endl;

    fout << CurrentTick << std::endl;

    fout << MyId << std::endl;

    fout << UnitsById.size() << std::endl;
    for (const auto& [_, unit]: UnitsById) {
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
    UnitsById = {};

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
        UnitsById[unit.PlayerId] = unit;
    }
}

}
