#include <fstream>
#include <iostream>

#include "emulator/World.h"

int main() {
    int emulationSteps = 1000;
    std::ofstream fout("test.output");

    Emulator::TWorld world;
    world.Load("world_seed_2.bin");

    int myUnitId;
    for (const auto& [unitId, unit]: world.UnitsById) {
        if (unit.PlayerId == world.MyId) {
            myUnitId = unitId;
            break;
        }
    }

    for (int i = 0; i < emulationSteps; ++i) {
        fout << world.UnitsById[myUnitId].Position << std::endl;

        world.EmulateOrder(Emulator::TOrder{
            .UnitId = myUnitId,
            .TargetVelocity = Emulator::Vector2D{-Emulator::GetGlobalConstants()->maxUnitForwardSpeed, 0},
        });
    }
}