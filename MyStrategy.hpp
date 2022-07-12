#ifndef _MY_STRATEGY_HPP_
#define _MY_STRATEGY_HPP_

#include "DebugInterface.hpp"
#include "model/Game.hpp"
#include "model/Order.hpp"
#include "model/Constants.hpp"
#include "emulator/public.h"

class MyStrategy {
public:
    MyStrategy(const model::Constants& constants);
    static model::Order getOrder(const model::Game& game, DebugInterface* debugInterface);
    static model::UnitOrder getUnitOrder(const model::Game& game, DebugInterface* debugInterface, const model::Unit& unit);
    void debugUpdate(DebugInterface& debugInterface);
    void finish();
};

#endif