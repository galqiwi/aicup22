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
    static std::vector<Emulator::TStrategy> bestStrategies;
    SetGlobalDebugInterface(debugInterface);

    int actionDuration = (int)lround(Emulator::GetGlobalConstants()->ticksPerSecond) / 2;
    int nActions = 5;
    int nStrategies = 100;
    int nMutations = 5;

    Emulator::TWorld world = Emulator::TWorld::FormApi(game);

    std::optional<double> bestScore = std::nullopt;
    Emulator::TStrategy bestStrategy;

    for (int i = 0; i < nStrategies; ++i) {
        Emulator::TStrategy strategy;

        if (i < bestStrategies.size()) {
            strategy = std::move(bestStrategies[i]);
        } else {
            strategy = Emulator::GenerateRandomStrategy(world.CurrentTick, actionDuration, nActions);
        }
        auto score = Emulator::EvaluateStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);

        if (!bestScore || *bestScore > score) {
            bestScore = score;
            bestStrategy = std::move(strategy);
        }
    }

//    debugInterface->addPlacedText(unit.position, std::to_string(*bestScore), model::Vec2{1, 0}, 2, debugging::Color{0, 0, 0, 1});
//    Emulator::VisualiseStrategy(bestStrategy, world, unit.id, world.CurrentTick + nActions * actionDuration);

    auto order = bestStrategy.GetOrder(world, unit.id);

    bestStrategies.resize(0);
    for (int i = 0; i < nMutations; ++i) {
        bestStrategies.push_back(bestStrategy.Mutate());
    }
    bestStrategies.push_back(std::move(bestStrategy));

    return order.ToApi();
}

void MyStrategy::debugUpdate(DebugInterface& debugInterface) {}

void MyStrategy::finish() {}