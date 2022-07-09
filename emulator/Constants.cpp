#include "Constants.h"

#include <memory>

namespace Emulator {

std::unique_ptr<TConstants> GlobalConstants;

TConstantsPtr GetGlobalConstants() {
    return GlobalConstants.get();
}

void SetGlobalConstants(TConstants constants) {
    GlobalConstants = std::make_unique<TConstants>(std::move(constants));
}

TConstants TConstants::FromAPI(const model::Constants &apiConstants) {
    std::vector<TObstacle> obstacles;
    obstacles.reserve(apiConstants.obstacles.size());

    for (const auto& obstacle: apiConstants.obstacles) {
        obstacles.push_back(TObstacle{
            Vector2D{obstacle.position.x, obstacle.position.y},
            obstacle.radius});
    }

    auto ticksPerSecond = apiConstants.ticksPerSecond;
    auto teamSize = apiConstants.teamSize;
    auto initialZoneRadius = apiConstants.initialZoneRadius;
    auto zoneSpeed = apiConstants.zoneSpeed;
    auto zoneDamagePerSecond = apiConstants.zoneDamagePerSecond;
    auto spawnTime = apiConstants.spawnTime;
    auto spawnCollisionDamagePerSecond = apiConstants.spawnCollisionDamagePerSecond;
    auto lootingTime = apiConstants.lootingTime;
    auto botPlayers = apiConstants.botPlayers;
    auto unitRadius = apiConstants.unitRadius;
    auto unitHealth = apiConstants.unitHealth;
    auto healthRegenerationPerSecond = apiConstants.healthRegenerationPerSecond;
    auto healthRegenerationDelay = apiConstants.healthRegenerationDelay;
    auto maxShield = apiConstants.maxShield;
    auto spawnShield = apiConstants.spawnShield;
    auto extraLives = apiConstants.extraLives;
    auto lastRespawnZoneRadius = apiConstants.lastRespawnZoneRadius;
    auto fieldOfView = apiConstants.fieldOfView;
    auto viewDistance = apiConstants.viewDistance;
    auto viewBlocking = apiConstants.viewBlocking;
    auto rotationSpeed = apiConstants.rotationSpeed;
    auto spawnMovementSpeed = apiConstants.spawnMovementSpeed;
    auto maxUnitForwardSpeed = apiConstants.maxUnitForwardSpeed;
    auto maxUnitBackwardSpeed = apiConstants.maxUnitBackwardSpeed;
    auto unitAcceleration = apiConstants.unitAcceleration;
    auto friendlyFire = apiConstants.friendlyFire;
    auto killScore = apiConstants.killScore;
    auto damageScoreMultiplier = apiConstants.damageScoreMultiplier;
    auto scorePerPlace = apiConstants.scorePerPlace;
    auto startingWeaponAmmo = apiConstants.startingWeaponAmmo;
    auto maxShieldPotionsInInventory = apiConstants.maxShieldPotionsInInventory;
    auto shieldPerPotion = apiConstants.shieldPerPotion;
    auto shieldPotionUseTime = apiConstants.shieldPotionUseTime;
    auto stepsSoundTravelDistance = apiConstants.stepsSoundTravelDistance;

    return {
        .obstacles = std::move(obstacles),
        .ticksPerSecond = ticksPerSecond,
        .teamSize = teamSize,
        .initialZoneRadius = initialZoneRadius,
        .zoneSpeed = zoneSpeed,
        .zoneDamagePerSecond = zoneDamagePerSecond,
        .spawnTime = spawnTime,
        .spawnCollisionDamagePerSecond = spawnCollisionDamagePerSecond,
        .lootingTime = lootingTime,
        .botPlayers = botPlayers,
        .unitRadius = unitRadius,
        .unitHealth = unitHealth,
        .healthRegenerationPerSecond = healthRegenerationPerSecond,
        .healthRegenerationDelay = healthRegenerationDelay,
        .maxShield = maxShield,
        .spawnShield = spawnShield,
        .extraLives = extraLives,
        .lastRespawnZoneRadius = lastRespawnZoneRadius,
        .fieldOfView = fieldOfView,
        .viewDistance = viewDistance,
        .viewBlocking = viewBlocking,
        .rotationSpeed = rotationSpeed,
        .spawnMovementSpeed = spawnMovementSpeed,
        .maxUnitForwardSpeed = maxUnitForwardSpeed,
        .maxUnitBackwardSpeed = maxUnitBackwardSpeed,
        .unitAcceleration = unitAcceleration,
        .friendlyFire = friendlyFire,
        .killScore = killScore,
        .damageScoreMultiplier = damageScoreMultiplier,
        .scorePerPlace = scorePerPlace,
        .startingWeaponAmmo = startingWeaponAmmo,
        .maxShieldPotionsInInventory = maxShieldPotionsInInventory,
        .shieldPerPotion = shieldPerPotion,
        .shieldPotionUseTime = shieldPotionUseTime,
        .stepsSoundTravelDistance = stepsSoundTravelDistance,
    };
}

}
