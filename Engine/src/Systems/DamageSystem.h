#pragma once

#include "../ECS/ECS.h"

#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"

class DamageSystem : public System
{
public:
    DamageSystem() {
        RequireComponent<BoxColliderComponent>();
    }

    void SubscribeToEvents(const std::unique_ptr<EventBus>& event_bus) {
        event_bus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::OnCollision);
    }

    void OnCollision(CollisionEvent& event) {
        Logger::Log("Collision event between entities: " + std::to_string(event.a.GetId()) + ", " + std::to_string(event.b.GetId()));;
        Entity a = event.a;
        Entity b = event.b;

        if (a.BelongsToGroup("projectiles") && b.HasTag("player")) {
            OnProjectileHitsPlayer(a, b);
        }
        if (b.BelongsToGroup("projectiles") && a.HasTag("player")) {
            OnProjectileHitsPlayer(b, a);
        }

        if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {
            OnProjectileHitsEnemy(a, b);
        }
        if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {
            OnProjectileHitsEnemy(b, a);
        }
    }

    void OnProjectileHitsPlayer(Entity projectile, Entity player) {
        auto& projectile_component = projectile.GetComponent<ProjectileComponent>();
        if (!projectile_component.is_friendly) {
            // Reduce the health of the player by the projectile hit percent damage.
            auto& health = player.GetComponent<HealthComponent>();
            // Subtract the health of the player.
            health.health_percentage -= projectile_component.hit_percent_damage;
            // Kills the player when health reaches zero.
            if (health.health_percentage <= 0) {
                player.Kill();
            }
            projectile.Kill();
        }
    }

    void OnProjectileHitsEnemy(Entity projectile, Entity enemy) {
        auto& projectile_component = projectile.GetComponent<ProjectileComponent>();
        if (projectile_component.is_friendly) {
            // Reduce the health of the player by the projectile hit percent damage.
            auto& health = enemy.GetComponent<HealthComponent>();
            // Subtract the health of the player.
            health.health_percentage -= projectile_component.hit_percent_damage;
            // Kills the player when health reaches zero.
            if (health.health_percentage <= 0) {
                enemy.Kill();
            }
            projectile.Kill();
        }
    }

    void Update() {

    }
};