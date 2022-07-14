#pragma once

#include "public.h"

#include "Vector2D.h"
#include "model/Sound.hpp"

namespace Emulator {

struct TSound {
    // Sound type index (starting with 0)
    int TypeIndex;
    // Id of unit that heard this sound
    int UnitId;
    // Position where sound was heard (different from sound source position)
    Vector2D Position;

    static TSound FromApi(model::Sound sound);
};

}