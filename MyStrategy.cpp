#include "MyStrategy.hpp"
#include <cassert>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>

#include "emulator/Constants.h"
#include "emulator/DebugSingleton.h"
#include "emulator/Evaluation.h"
#include "emulator/LootPicker.h"
#include "emulator/Memory.h"
#include "emulator/Sound.h"
#include "emulator/World.h"

MyStrategy::MyStrategy(const model::Constants& constants) {
    Emulator::SetGlobalConstants(Emulator::TConstants::FromAPI(constants));
}

model::Order MyStrategy::doGetOrder(const model::Game& game, DebugInterface* debugInterface) {
    std::unordered_map<int, model::UnitOrder> actions;
    for (auto &unit : game.units)
    {
        if (unit.playerId != game.myId)
            continue;

        actions.insert({unit.id, getUnitOrder(game, debugInterface, unit)});
    }

    return {actions};
}

model::Order MyStrategy::getOrder(const model::Game& game, DebugInterface* debugInterface) {
    static auto globalStart = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    auto output = doGetOrder(game, debugInterface);
    auto finish = std::chrono::high_resolution_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(finish-globalStart).count() / 1000. << "\t";
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(finish-start).count() / 1000. << "\n";

    return output;
}

model::UnitOrder MyStrategy::getUnitOrder(const model::Game& game, DebugInterface* debugInterface, const model::Unit& unit) {
    static auto constants = Emulator::GetGlobalConstants();
    static std::unordered_map<int, std::vector<Emulator::TStrategy>> forcedStrategiesById;
    static Emulator::TMemory memory;
    auto& forcedStrategies = forcedStrategiesById[unit.id];

    SetGlobalDebugInterface(debugInterface);

    int actionDuration = (int)lround(Emulator::GetGlobalConstants()->ticksPerSecond) / 2;
    int nActions = 5;
    int nStrategies = 100;
    int nMutations = 5;

    Emulator::TWorld world = Emulator::TWorld::FormApi(game);
    memory.Update(world);
    for (const auto& sound: game.sounds) {
        memory.UpdateSoundKnowledge(world, Emulator::TSound::FromApi(sound));
    }

    memory.InjectKnowledge(world);
    world.UpdateLootIndex();
    world.UpdateUnitsTargetLoot();

    {
        auto& newState = world.StateByUnitId[unit.id];
        newState.Sync(world);
    }

    std::optional<Emulator::TScore> bestScore = std::nullopt;
    std::optional<Emulator::TStrategy> bestStrategy;

    std::vector<int> projectileIds;
    for (const auto& [projectileId, _]: world.ProjectileById) {
        projectileIds.push_back(projectileId);
    }

    for (const auto& [_, projectile]: world.ProjectileById) {
        if (!SegmentIntersectsCircle(projectile.Position, projectile.Position + projectile.Velocity * projectile.LifeTime, Emulator::Vector2D::FromApi(unit.position), constants->unitRadius)) {
            continue;
        }
        auto direction = Emulator::rot90(projectile.Velocity);
        forcedStrategies.push_back(Emulator::GenerateRunaway(direction));
        forcedStrategies.push_back(Emulator::GenerateRunaway(direction * -1));
    }

    forcedStrategies.push_back(Emulator::TStrategy{
        .GoTo = Emulator::GetTarget(world, unit.id, false),
    });

    forcedStrategies.push_back(Emulator::TStrategy{
        .GoTo = Emulator::GetTarget(world, unit.id, true),
    });


    static int64_t globalTimeResource = 0;
    int64_t microsecondsToGo = 30000 / constants->teamSize;
    globalTimeResource += microsecondsToGo;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0;; ++i) {
        if (i >= nStrategies + forcedStrategies.size()) {
//            std::cerr << i << "\n";
            break;
        }

        if (i > forcedStrategies.size() && std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start).count() > globalTimeResource) {
//            std::cerr << i << " " << forcedStrategies.size() << "\n";
            break;
        }

        Emulator::TStrategy strategy;

        if (i < forcedStrategies.size()) {
            strategy = forcedStrategies[i];
        } else {
            strategy = Emulator::GenerateRandomStrategy(world.CurrentTick, actionDuration, nActions);
        }
        auto score = Emulator::EvaluateStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);
//        Emulator::VisualiseStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);

        if (!bestScore || score < *bestScore) {
            bestScore = score;
            bestStrategy = std::move(strategy);
        }
    }

//    for (const auto& sound: game.sounds) {
//        debugInterface->addCircle(sound.position, 0.25, debugging::Color(1, 0, 1, 1));
//    }
//    Emulator::EvaluateStrategy(forcedStrategies[0], world, unit.id, world.CurrentTick + nActions * actionDuration);
//    if (!forcedStrategies.empty()) {
//        Emulator::VisualiseStrategy(forcedStrategies[0], world, unit.id, world.CurrentTick + nActions * actionDuration);
//    }
//
//    Emulator::VisualiseStrategy(bestStrategy, world, unit.id, world.CurrentTick + nActions * actionDuration);
//    debugInterface->addPlacedText(unit.position, std::to_string(std::get<0>(*bestScore)), model::Vec2{1, 0}, 2, debugging::Color{0, 0, 0, 1});
//    for (const auto& [_, unitToDraw]: world.UnitById) {
//        debugInterface->addCircle(unitToDraw.Position.ToApi(), 0.6, debugging::Color(0, 1, 1, 1));
//    }
//    debugInterface->addCircle(GetTarget(world, unit.id).ToApi(), 0.25, debugging::Color(1, 0, 1, 1));
//    for (const auto& [_, loot]: world.LootById) {
//        debugInterface->addCircle(loot.Position.ToApi(), 0.6, debugging::Color(1, 0, 1, 1));
//    }
//
//    for (const auto& [_, otherUnit]: world.UnitById) {
//        auto color = debugging::Color(0, 1, 0, 1);
//        if (Emulator::GetGlobalConstants()->obstaclesMeta.SegmentIntersectsObstacle(otherUnit.Position, Emulator::Vector2D::FromApi(unit.position))) {
//            color = debugging::Color(1, 0, 0, 1);
//        }
//        debugInterface->addPolyLine({unit.position, otherUnit.Position.ToApi()}, 0.1, color);
//    }

//    debugInterface->addCircle(unit.position, 0.9, world.PreprocessedDataById[unit.id].InDanger ? debugging::Color(1, 0, 0, 1):debugging::Color(0, 1, 0, 1));

    assert(bestStrategy);
    assert(bestScore);


    // TODO: test this
    if (bestScore->HealthScore > (constants->unitHealth - unit.health) * nActions * actionDuration + 1e-6) {
        for (auto strategy: forcedStrategies) {
            strategy.ObedienceLevel = Emulator::VERY_SOFT;
            auto score = Emulator::EvaluateStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);

            if (score.HealthScore >= bestScore->HealthScore - 1e-6) {
                continue;
            }

            if (score < *bestScore) {
                bestScore = score;
                bestStrategy = std::move(strategy);
            }
        }
    }

//    if (bestScore->HealthScore > (constants->unitHealth - unit.health) * nActions * actionDuration + 1e-6) {
////        debugInterface->addCircle(unit.position, 0.9, debugging::Color(1, 0, 0, 1));
//        for (auto strategy: forcedStrategies) {
//            strategy.ObedienceLevel = Emulator::SOFT;
//            auto score = Emulator::EvaluateStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);
//
//            if (score.HealthScore >= bestScore->HealthScore - 1e-6) {
//                continue;
//            }
//
//            if (score < *bestScore) {
//                bestScore = score;
//                bestStrategy = std::move(strategy);
//            }
//        }
//    }

//    if (bestScore->HealthScore > (constants->unitHealth - unit.health) * nActions * actionDuration + 1e-6) {
////        debugInterface->addCircle(unit.position, 0.9, debugging::Color(1, 1, 0, 1));
//        for (auto strategy: forcedStrategies) {
//            strategy.ObedienceLevel = Emulator::HARD;
//            auto score = Emulator::EvaluateStrategy(strategy, world, unit.id, world.CurrentTick + nActions * actionDuration);
//
//            if (score.HealthScore >= bestScore->HealthScore - 1e-6) {
//                continue;
//            }
//
//            if (score < *bestScore) {
//                bestScore = score;
//                bestStrategy = std::move(strategy);
//            }
//        }
//    }

    auto order = bestStrategy->GetOrder(world, unit.id, /*forSimulation*/ false);

    {
        auto newState = world.StateByUnitId[unit.id];
        newState.Update(world, order);
        memory.RememberState(unit.id, newState);
    }

    if (order.Pickup && unit.aim < 1e-4) {
        memory.ForgetLoot(order.LootId);
    }

    forcedStrategies.resize(0);
    forcedStrategies.push_back(*bestStrategy);
    for (int i = 0; i < nMutations; ++i) {
        forcedStrategies.push_back(bestStrategy->Mutate());
    }

    globalTimeResource -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start).count();

    return order.ToApi();
}

void MyStrategy::debugUpdate(DebugInterface& debugInterface) {}

void MyStrategy::finish() {}