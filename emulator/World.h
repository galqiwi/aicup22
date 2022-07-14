#pragma once

#include "public.h"
#include "Vector2D.h"
#include "Constants.h"

#include "model/Game.hpp"
#include "model/UnitOrder.hpp"

#include <unordered_map>
#include <vector>


namespace Emulator {

struct TUnit {
    int Id;
    int PlayerId;
    Vector2D Position;
    Vector2D Direction;
    Vector2D Velocity;
    double Health;
    double Shield;
    int ExtraLives;
    std::optional<double> RemainingSpawnTime;
    double Aim;
    int HealthRegenerationStartTick;
    std::optional<int> Weapon;
    int NextShotTick;
    std::vector<int> Ammo;
    int ShieldPotions;

    bool Imaginable{false};

    double GetCombatRadius() const;
};

struct TOrder {
    int UnitId;
    Vector2D TargetVelocity;
    Vector2D TargetDirection;
    bool Shoot{false};
    bool Pickup{false};
    int LootId{-1};
    bool UseShieldPotion{false};
    bool IsRotationStart{false};

    [[nodiscard]] model::UnitOrder ToApi() const;
};

struct TZone {
    Vector2D currentCenter;
    double currentRadius;
    Vector2D nextCenter;
    double nextRadius;
};

struct TProjectile {
    int Id;
    int WeaponTypeIndex;
    int ShooterId;
    int ShooterPlayerId;
    Vector2D Position;
    Vector2D Velocity;
    double LifeTime;
};

enum ELootItem {
    Weapon = 0,
    ShieldPotions = 1,
    Ammo = 2,
};

struct TLoot {
    int Id;
    Vector2D Position;
    ELootItem Item;
    int WeaponType;
    int Amount;
};

struct TState {
    void Update(const TWorld& world, const TOrder& order);

    // TODO: by Id
    int LastRotationTick{0};

};

class TWorld {
public:
    void Emulate(const std::vector<TOrder>& orders);
    static TWorld FormApi(const model::Game& game);

    void Dump(const char* filename);
    void Load(const char* filename);

    int MyId;
    int CurrentTick;
    std::unordered_map<int, TUnit> UnitById;
    TZone Zone;
    std::unordered_map<int, TProjectile> ProjectileById;
    std::unordered_map<int, TLoot> LootById;
    std::unordered_map<int, std::vector<TLoot>> LootByItemIndex;
    TState State;

    void PrepareEmulation();
    void EmulateOrder(const TOrder& order);
    void Tick();
    void UpdateLootIndex();
private:
    Vector2D ClipVelocity(Vector2D velocity, const TUnit& unit);
    Vector2D ApplyAcceleration(Vector2D velocity, Vector2D targetVelocity);
    void MoveCollidingUnit(TUnit& unit, Vector2D velocity);
    void RotateUnit(TUnit& unit, Vector2D targetDirection);

    TConstantsPtr Constants_ = nullptr;
};

}
