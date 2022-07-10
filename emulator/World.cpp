#include <cassert>
#include <fstream>
#include <iostream>

#include "World.h"

namespace Emulator {

TWorld TWorld::FormApi(const model::Game& game) {
    TWorld output;
    for (const auto& unit: game.units) {
        output.UnitsById[unit.id] = TUnit{
            .Id = unit.id,
            .PlayerId = unit.playerId,
            .Position = Vector2D::FromApi(unit.position),
            .Direction = Vector2D::FromApi(unit.direction),
            .Velocity = Vector2D::FromApi(unit.velocity),
        };
    }
    output.MyId = game.myId;

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

    for (const auto& obstacle: Constants_->obstacles) {
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

const std::string LOAD_DUMP_VERSION = "5.0";

void TWorld::Dump(const char *filename) {
    std::ofstream fout(filename);
    fout.precision(20);

    fout << LOAD_DUMP_VERSION << std::endl;

    fout << *GetGlobalConstants() << std::endl;

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
