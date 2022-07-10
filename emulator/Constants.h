#pragma once

#include "Vector2D.h"

#include "model/Constants.hpp"

#include <vector>


namespace Emulator {

struct TObstacle {
    Vector2D Center;
    double Radius;
};

struct TConstants {
    std::vector<TObstacle> obstacles;

    // Number of ticks per game second
    double ticksPerSecond;
    // Starting number of units in each team
    int teamSize;
    // Initial zone radius
    double initialZoneRadius;
    // Speed of zone radius
    double zoneSpeed;
    // Damage dealt to units outside of the zone per second
    double zoneDamagePerSecond;
    // Unit spawning time
    double spawnTime;
    // Damage dealt to units trying to spawn in incorrect position per second
    double spawnCollisionDamagePerSecond;
    // Time required to perform looting actions (in seconds)
    double lootingTime;
    // Number of bot players (teams)
    int botPlayers;
    // Units' radius
    double unitRadius;
    // Max units' health
    double unitHealth;
    // Health automatically restored per second
    double healthRegenerationPerSecond;
    // Time until automatic health regeneration since last health damage (in seconds)
    double healthRegenerationDelay;
    // Max value of unit's shield
    double maxShield;
    // Initial value of unit's shield
    double spawnShield;
    // Initial number of extra lives for units
    int extraLives;
    // Zone radius after which respawning is disabled
    double lastRespawnZoneRadius;
    // Units' field of view without aiming (in degrees)
    double fieldOfView;
    // Units' view distance
    double viewDistance;
    // Whether units' view is blocked by obstacles
    bool viewBlocking;
    // Unit rotation speed without aiming (degrees per second)
    double rotationSpeed;
    // Units' movement speed while spawning
    double spawnMovementSpeed;
    // Max unit speed when walking forward
    double maxUnitForwardSpeed;
    // Max unit speed when walking backward
    double maxUnitBackwardSpeed;
    // Max unit acceleration
    double unitAcceleration;
    // Whether a unit can damage units of the same team
    bool friendlyFire;
    // Score given for killing enemy unit
    double killScore;
    // Score multiplier for damaging enemy units
    double damageScoreMultiplier;
    // Score given for every team killed before you
    double scorePerPlace;
    // Ammo for starting weapon given when unit spawns
    int startingWeaponAmmo;
    // Max number of shield potions in unit's inventory
    int maxShieldPotionsInInventory;
    // Amount of shield restored using one potion
    double shieldPerPotion;
    // Time required to perform action of using shield potion
    double shieldPotionUseTime;
    // Distance when steps sound will be 100% probability
    double stepsSoundTravelDistance;

    static TConstants FromAPI(const model::Constants& apiConstants);
};

using TConstantsPtr = TConstants*;

TConstantsPtr GetGlobalConstants();
void SetGlobalConstants(TConstants obstacles);

std::ostream& operator<<(std::ostream& out, const TConstants& c);

std::istream& operator>>(std::istream& in, TConstants& c);

}
