#include "emulator/Vector2D.h"

#include <iostream>

int main() {
    srand(239);
    for (int i = 0; i < 10; ++i) {
        std::cout << Emulator::RandomUniformVector() << std::endl;
    }
}
