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

std::ostream& operator<<(std::ostream& out, const TConstants& c) {
    out << c.obstacles.size() << std::endl;
    for (const auto& obstacle: c.obstacles) {
        out << obstacle.Radius << std::endl;
        out << obstacle.Center << std::endl;
    }

    out << c.ticksPerSecond << std::endl;
    out << c.teamSize << std::endl;
    out << c.initialZoneRadius << std::endl;
    out << c.zoneSpeed << std::endl;
    out << c.zoneDamagePerSecond << std::endl;
    out << c.spawnTime << std::endl;
    out << c.spawnCollisionDamagePerSecond << std::endl;
    out << c.lootingTime << std::endl;
    out << c.botPlayers << std::endl;
    out << c.unitRadius << std::endl;
    out << c.unitHealth << std::endl;
    out << c.healthRegenerationPerSecond << std::endl;
    out << c.healthRegenerationDelay << std::endl;
    out << c.maxShield << std::endl;
    out << c.spawnShield << std::endl;
    out << c.extraLives << std::endl;
    out << c.lastRespawnZoneRadius << std::endl;
    out << c.fieldOfView << std::endl;
    out << c.viewDistance << std::endl;
    out << c.viewBlocking << std::endl;
    out << c.rotationSpeed << std::endl;
    out << c.spawnMovementSpeed << std::endl;
    out << c.maxUnitForwardSpeed << std::endl;
    out << c.maxUnitBackwardSpeed << std::endl;
    out << c.unitAcceleration << std::endl;
    out << c.friendlyFire << std::endl;
    out << c.killScore << std::endl;
    out << c.damageScoreMultiplier << std::endl;
    out << c.scorePerPlace << std::endl;
    out << c.startingWeaponAmmo << std::endl;
    out << c.maxShieldPotionsInInventory << std::endl;
    out << c.shieldPerPotion << std::endl;
    out << c.shieldPotionUseTime << std::endl;
    out << c.stepsSoundTravelDistance;

    return out;
}

std::istream& operator>>(std::istream& in, TConstants& c) {
    int obstaclesSize;
    in >> obstaclesSize;
    c.obstacles.reserve(obstaclesSize);
    for (int i = 0; i < obstaclesSize; ++i) {
        TObstacle obstacle{};
        in >> obstacle.Radius;
        in >> obstacle.Center;
        c.obstacles.push_back(obstacle);
    }

    in >> c.ticksPerSecond;
    in >> c.teamSize;
    in >> c.initialZoneRadius;
    in >> c.zoneSpeed;
    in >> c.zoneDamagePerSecond;
    in >> c.spawnTime;
    in >> c.spawnCollisionDamagePerSecond;
    in >> c.lootingTime;
    in >> c.botPlayers;
    in >> c.unitRadius;
    in >> c.unitHealth;
    in >> c.healthRegenerationPerSecond;
    in >> c.healthRegenerationDelay;
    in >> c.maxShield;
    in >> c.spawnShield;
    in >> c.extraLives;
    in >> c.lastRespawnZoneRadius;
    in >> c.fieldOfView;
    in >> c.viewDistance;
    in >> c.viewBlocking;
    in >> c.rotationSpeed;
    in >> c.spawnMovementSpeed;
    in >> c.maxUnitForwardSpeed;
    in >> c.maxUnitBackwardSpeed;
    in >> c.unitAcceleration;
    in >> c.friendlyFire;
    in >> c.killScore;
    in >> c.damageScoreMultiplier;
    in >> c.scorePerPlace;
    in >> c.startingWeaponAmmo;
    in >> c.maxShieldPotionsInInventory;
    in >> c.shieldPerPotion;
    in >> c.shieldPotionUseTime;
    in >> c.stepsSoundTravelDistance;

    return in;
}

}
