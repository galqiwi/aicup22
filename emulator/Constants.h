#pragma once

#include "Vector2D.h"

#include "model/Constants.hpp"

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>

struct hash_pair {
    size_t operator()(const std::pair<int, int>& p) const
    {
        auto a = p.first;
        auto b = p.second;
        auto A = (ulong)(a >= 0 ? 2 * (long)a : -2 * (long)a - 1);
        auto B = (ulong)(b >= 0 ? 2 * (long)b : -2 * (long)b - 1);
        auto C = (long)((A >= B ? A * A + A + B : A + B * B) / 2);
        return a < 0 && b < 0 || a >= 0 && b >= 0 ? C : -C - 1;
    }
};

namespace Emulator {

struct TObstacle {
    Vector2D Center;
    double Radius;
    bool CanSeeThrough;
    bool CanShootThrough;
};

class TObstacleMeta {
public:
    TObstacleMeta();
    explicit TObstacleMeta(const std::vector<TObstacle>& obstacles);

    const std::vector<int>& GetIntersectingIds(Vector2D point);
    std::optional<int> GetObstacle(Vector2D point);
    bool SegmentIntersectsObstacle(Vector2D p1, Vector2D p2);

    bool IsInitialized() const;
private:
    bool Initialized_ = false;

    std::vector<TObstacle> Obstacles_;
    std::unordered_map<std::pair<int, int>, std::vector<int>, hash_pair> Index_;
    std::vector<int> EmptyList_;

    bool SubSegmentIntersectsObstacle(Vector2D p1, Vector2D p2, std::unordered_set<int>& obstacles);
    bool SegmentIntersectsObstacleNearPoint(Vector2D p1, Vector2D p2, Vector2D p, std::unordered_set<int>& obstacles);
};

struct TConstants {
    std::vector<TObstacle> obstacles;
    TObstacleMeta obstaclesMeta;

    double realTicksPerSecond;
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
    // List of properties of every weapon type
    std::vector<model::WeaponProperties> weapons;
    std::vector<model::SoundProperties> sounds;

    static TConstants FromAPI(const model::Constants& apiConstants);
};

using TConstantsPtr = TConstants*;

TConstantsPtr GetGlobalConstants();
void SetGlobalConstants(TConstants obstacles);

std::ostream& operator<<(std::ostream& out, const TConstants& c);

std::istream& operator>>(std::istream& in, TConstants& c);

}
