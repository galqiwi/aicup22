#include "DebugSingleton.h"

DebugInterface* GlobalDebugInterface;

void SetGlobalDebugInterface(DebugInterface* value) {
    GlobalDebugInterface = value;
}

DebugInterface* GetGlobalDebugInterface() {
    return GlobalDebugInterface;
}
