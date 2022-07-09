#include "MyStrategy.hpp"
#include <exception>

#include "emulator/Constants.h"
#include "emulator/World.h"

MyStrategy::MyStrategy(const model::Constants& constants) {
    Emulator::SetGlobalConstants(Emulator::TConstants::FromAPI(constants));
}

model::Order MyStrategy::getOrder(const model::Game& game, DebugInterface* debugInterface)
{
    Emulator::TWorld world = Emulator::TWorld::FormApi(game);
    world.Dump("world_seed_2.bin");
    abort();

    std::unordered_map<int, model::UnitOrder> actions;
    for (auto &unit : game.units)
    {
        if (unit.playerId != game.myId)
            continue;

        std::shared_ptr<model::ActionOrder::Aim> aim = std::make_shared<model::ActionOrder::Aim>(false);
        std::optional<std::shared_ptr<model::ActionOrder>> action = std::make_optional(aim);

        model::UnitOrder order({-1, 0}, {unit.direction.x, unit.direction.y}, std::nullopt);
        actions.insert({unit.id, order});
    }
    return model::Order(actions);
}

void MyStrategy::debugUpdate(DebugInterface& debugInterface) {}

void MyStrategy::finish() {}