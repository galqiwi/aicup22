#pragma once

#include <memory>

namespace Emulator {

struct TUnit;

struct TOrder;

class TWorld;

struct Vector2D;

struct TStrategyAction;

struct TStrategy;

struct TObstacle;

class TObstacleMeta;

struct TConstants;

struct TMemory;

struct TLoot;

struct TState;

struct IClosestPointIndex;

using IClosestPointIndexPtr = std::shared_ptr<IClosestPointIndex>;

enum ELootItem {
    Weapon = 0,
    ShieldPotions = 1,
    Ammo = 2,
};

}