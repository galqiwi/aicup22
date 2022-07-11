#include "MyStrategy.hpp"
#include <exception>
#include <fstream>
#include <iostream>

#include "emulator/Constants.h"
#include "emulator/DebugSingleton.h"
#include "emulator/Evaluation.h"
#include "emulator/World.h"

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
    SetGlobalDebugInterface(debugInterface);

    int actionDuration = 15;
    int nActions = 5;
    int nStrategies = 100;

    Emulator::TWorld world = Emulator::TWorld::FormApi(game);

    std::optional<double> bestScore = std::nullopt;
    Emulator::TStrategy bestStrategy;

    for (int i = 0; i < nStrategies; ++i) {
        auto strategy = Emulator::GenerateRandomStrategy(world.CurrentTick, actionDuration, nActions);
        auto score = Emulator::EvaluateStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);

        if (!bestScore || *bestScore < score) {
            bestScore = score;
            bestStrategy = std::move(strategy);
        }
    }

    return bestStrategy.GetOrder(world, unit.id).ToApi();
}

void MyStrategy::debugUpdate(DebugInterface& debugInterface) {}

void MyStrategy::finish() {}