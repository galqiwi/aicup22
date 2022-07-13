#pragma once

#include "public.h"

#include <optional>

namespace Emulator {

std::optional<int> GetTargetLoot(const TWorld &world, int unitId);

Vector2D GetTarget(const TWorld &world, std::optional<int> loot);
Vector2D GetTarget(const TWorld &world, int unitId);

}
