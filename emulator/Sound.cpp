#include "Sound.h"

namespace Emulator {

TSound TSound::FromApi(model::Sound sound) {
    return {
        .TypeIndex = sound.typeIndex,
        .UnitId = sound.unitId,
        .Position = Vector2D::FromApi(sound.position),
    };
}

}
