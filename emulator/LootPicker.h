#pragma once

#include "public.h"

#include <optional>

namespace Emulator {

std::optional<int> GetTargetLoot(const TWorld &world, int unitId, bool excludeDangerous);

}
