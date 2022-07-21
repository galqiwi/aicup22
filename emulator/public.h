#pragma once

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

enum EAutomatonState {
    RES_GATHERING = 0,
    FIGHT = 1,
};

}