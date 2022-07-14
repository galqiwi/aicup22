#include <fstream>
#include <iostream>

#include "emulator/World.h"

int main() {
    double a = 100.1;
    double b = a;
    std::cout << (a == a) << std::endl;

    abort();
    int emulationSteps = 201;
    std::ofstream fout("test.output");

    Emulator::TWorld world;
    world.Load("world_seed_2.bin");

    int myUnitId;
    for (const auto& [unitId, unit]: world.UnitById) {
        if (unit.PlayerId == world.MyId) {
            myUnitId = unitId;
            break;
        }
    }

    for (int i = 0; i < emulationSteps; ++i) {
        fout << world.UnitById[myUnitId].Position << std::endl;

        world.EmulateOrder(Emulator::TOrder{
            .UnitId = myUnitId,
            .TargetVelocity = Emulator::Vector2D{-1000, 0},
            .TargetDirection = Emulator::Vector2D{world.UnitById[myUnitId].Direction.y, -world.UnitById[myUnitId].Direction.x},
        });
    }
}