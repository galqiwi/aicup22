#include "MyStrategy.hpp"
#include <exception>
#include <fstream>
#include <iostream>

#include "emulator/Constants.h"
#include "emulator/DebugSingleton.h"
#include "emulator/Evaluation.h"
#include "emulator/World.h"
#include "emulator/LootPicker.h"
#include "emulator/Memory.h"
#include "emulator/Sound.h"

MyStrategy::MyStrategy(const model::Constants& constants) {
    Emulator::SetGlobalConstants(Emulator::TConstants::FromAPI(constants));
}

model::Order MyStrategy::getOrder(const model::Game& game, DebugInterface* debugInterface) {
    std::unordered_map<int, model::UnitOrder> actions;
    for (auto &unit : game.units)
    {
        if (unit.playerId != game.myId)
            continue;

        actions.insert({unit.id, getUnitOrder(game, debugInterface, unit)});
    }

    return {actions};
}

model::UnitOrder MyStrategy::getUnitOrder(const model::Game& game, DebugInterface* debugInterface, const model::Unit& unit) {
    auto world = Emulator::TWorld::FormApi(game);
    Emulator::TOrder order;
    if (world.CurrentTick < 400) {
        order = {
            .TargetVelocity = Emulator::Vector2D::FromApi(unit.position) * (-1),
            .TargetDirection = {-1, 0},
        };
    } else {
        order = {
            .TargetVelocity = Emulator::Vector2D::FromApi(unit.position) * (-1),
            .TargetDirection = {-1, 0},
            .Shoot = true,
        };
    }


    return order.ToApi();
}

void MyStrategy::debugUpdate(DebugInterface& debugInterface) {}

void MyStrategy::finish() {}