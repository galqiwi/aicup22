#include "Constants.h"

#include <cassert>
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
        obstacles.push_back({
        .Center = Vector2D{obstacle.position.x, obstacle.position.y},
        .Radius = obstacle.radius,
        .CanSeeThrough = obstacle.canSeeThrough,
        .CanShootThrough = obstacle.canShootThrough});
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
    auto weapons = apiConstants.weapons;
    auto sounds = apiConstants.sounds;

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
        .weapons = weapons,
        .sounds = sounds,
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

std::pair<int, int> ToCellId(Vector2D position) {
    return {position.x, position.y};
}

TObstacleMeta::TObstacleMeta(): Initialized_(false) {
}

TObstacleMeta::TObstacleMeta(const std::vector<TObstacle> &obstacles): Initialized_(true), Obstacles_(obstacles) {
    for (int i = 0; i < obstacles.size(); ++i) {
        auto [xMin, yMin] = ToCellId(
            obstacles[i].Center - Vector2D{1, 1} * (obstacles[i].Radius + GetGlobalConstants()->unitRadius));

        auto [xMax, yMax] = ToCellId(
            obstacles[i].Center + Vector2D{1, 1} * (obstacles[i].Radius + GetGlobalConstants()->unitRadius));

        for (int x = xMin; x <= xMax; ++x) {
            for (int y = yMin; y <= yMax; ++y) {
                Index_[{x, y}].push_back(i);
            }
        }
    }
}

bool TObstacleMeta::IsInitialized() const {
    return Initialized_;
}

const std::vector<int>& TObstacleMeta::GetIntersectingIds(Vector2D point) {
    auto it = Index_.find(ToCellId(point));
    if (it == Index_.end()) {
        return EmptyList_;
    }

    return it->second;
}

std::optional<int> TObstacleMeta::GetObstacle(Vector2D point) {
    for (auto& id: GetIntersectingIds(point)) {
        auto obstacle = Obstacles_[id];
        if (abs2(obstacle.Center - point) < obstacle.Radius * obstacle.Radius) {
            return id;
        }
    }
    return std::nullopt;
}

bool TObstacleMeta::SegmentIntersectsObstacleNearPoint(Vector2D p1, Vector2D p2, Vector2D p, std::unordered_set<int>& obstacles) {
    auto constants = GetGlobalConstants();
    assert(constants);

    for (auto id: GetIntersectingIds(p)) {
        if (obstacles.contains(id)) {
            continue;
        }
        obstacles.insert(id);
        auto& obstacle = constants->obstacles[id];
        if (obstacle.CanShootThrough) {
            continue;
        }
        if (SegmentIntersectsCircle(p1, p2, obstacle.Center, obstacle.Radius)) {
            return true;
        }
    }
    return false;
}

bool TObstacleMeta::SubSegmentIntersectsObstacle(Vector2D p1, Vector2D p2, std::unordered_set<int>& obstacles) {
    auto c1 = ToCellId(p1);
    auto c2 = ToCellId(p2);
    Vector2D m = (p1 + p2) / 2;
    auto cm = ToCellId(m);

    if (c1 != cm && cm != c2) {
        if (SegmentIntersectsObstacleNearPoint(p1, p2, m, obstacles)) {
            return true;
        }
    }

    if (c1 == cm || cm == c2) {
        return false;
    }

    if (SubSegmentIntersectsObstacle(p1, m, obstacles)) {
        return true;
    }
    if (SubSegmentIntersectsObstacle(m, p2, obstacles)) {
        return true;
    }

    return false;
}

bool TObstacleMeta::SegmentIntersectsObstacle(Vector2D p1, Vector2D p2) {
    auto constants = GetGlobalConstants();
    assert(constants);
    std::unordered_set<int> obstacles;

    if (SegmentIntersectsObstacleNearPoint(p1, p2, p1, obstacles)) {
        return true;
    }
    if (SegmentIntersectsObstacleNearPoint(p1, p2, p2, obstacles)) {
        return true;
    }
    if (SubSegmentIntersectsObstacle(p1, p2, obstacles)) {
        return true;
    }

    return false;
}

}
