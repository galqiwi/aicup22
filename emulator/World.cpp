#include <cassert>
#include <fstream>

#include "World.h"

namespace Emulator {

TWorld TWorld::FormApi(const model::Game& game) {
    TWorld output;
    for (const auto& unit: game.units) {
        output.UnitsById_[unit.id] = TUnit{
            .Position = Vector2D::FromApi(unit.position),
            .Direction = Vector2D::FromApi(unit.direction),
            .Velocity = Vector2D::FromApi(unit.velocity),
        };
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

    assert(UnitsById_.contains(unitId));

    auto &unit = UnitsById_[unitId];

    auto targetVelocity = ClipVelocity(order.TargetVelocity, unit);
    auto velocity = ApplyAcceleration(unit.Velocity, targetVelocity);

}

Vector2D TWorld::ClipVelocity(Vector2D velocity, const TUnit &unit) {
    if (abs2(velocity) < Constants_->maxUnitBackwardSpeed * Constants_->maxUnitBackwardSpeed) {
        return velocity;
    }

    auto projection = (norm(unit.Direction) * norm(velocity)) * (Constants_->maxUnitForwardSpeed - Constants_->maxUnitBackwardSpeed) / 2;

    auto limit = sqrt(projection * projection + Constants_->maxUnitBackwardSpeed * Constants_->maxUnitForwardSpeed) - projection;

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

void TWorld::Dump(const char *filename) {
    std::ofstream fout(filename);
    fout.precision(20);

    fout << UnitsById_.size() << std::endl;
    for (const auto& [_, unit]: UnitsById_) {
        fout << unit.PlayerID << std::endl;
        fout << unit.Position << std::endl;
        fout << unit.Direction << std::endl;
        fout << unit.Velocity << std::endl;
    }
}

void TWorld::Load(const char *filename) {
    std::ifstream fin(filename);
    fin.precision(20);
    UnitsById_ = {};

    int unitsByIdSize;
    fin >> unitsByIdSize;

    for (int i = 0; i < unitsByIdSize; ++i) {
        TUnit unit{};
        fin >> unit.PlayerID;
        fin >> unit.Position;
        fin >> unit.Direction;
        fin >> unit.Velocity;
        UnitsById_[unit.PlayerID] = unit;
    }
}

}
