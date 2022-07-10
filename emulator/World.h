#pragma once

#include "Vector2D.h"
#include "Constants.h"

#include "model/Game.hpp"

#include <unordered_map>
#include <vector>


namespace Emulator {

struct TUnit {
    int Id;
    int PlayerId;
    Vector2D Position;
    Vector2D Direction;
    Vector2D Velocity;
};

struct TOrder {
    int UnitId;
    Vector2D TargetVelocity;
};

class TWorld {
public:
    void Emulate(const std::vector<TOrder>& orders);
    static TWorld FormApi(const model::Game& game);

    void Dump(const char* filename);
    void Load(const char* filename);

    int MyId;
    std::unordered_map<int, TUnit> UnitsById;
    void EmulateOrder(const TOrder& order);

private:
    Vector2D ClipVelocity(Vector2D velocity, const TUnit& unit);
    Vector2D ApplyAcceleration(Vector2D velocity, Vector2D targetVelocity);
    void MoveCollidingUnit(TUnit& unit, Vector2D velocity);

    TConstantsPtr Constants_ = nullptr;
};

}
