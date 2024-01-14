#pragma once

#include <glm/glm.hpp>

#include "../ECS/ECS.h"

#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"

class MovementSystem : public System
{
public:
    MovementSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& event_bus) {
        event_bus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::OnCollision);
    }


    void OnCollision(CollisionEvent& event) {
        /*Logger::Log("Collision event between entities: " + std::to_string(event.a.GetId()) + ", " + std::to_string(event.b.GetId()));;*/
        Entity a = event.a;
        Entity b = event.b;

        if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles")) {
            OnEnemyHitsObstacle(a, b);
        }

        if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
            OnEnemyHitsObstacle(b, a);
        }
    }

    void OnEnemyHitsObstacle(Entity enemy, Entity obstacle) {
        if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>()) {
            auto& rigid_body = enemy.GetComponent<RigidBodyComponent>();
            auto& sprite = enemy.GetComponent<SpriteComponent>();

            if (rigid_body.velocity.x != 0) {
                rigid_body.velocity.x *= -1;
                sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
             }

            if (rigid_body.velocity.y != 0) {
                rigid_body.velocity.y *= -1;
                sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
            }
        }
    }

    void Update(double delta_time) {
        // Loop all entities that the system is interested in...
        for (auto entity : GetSystemEntities()) {
            // Update entity position based on its velocity.
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rigid_body = entity.GetComponent<RigidBodyComponent>();

            transform.position.x += rigid_body.velocity.x * delta_time;
            transform.position.y += rigid_body.velocity.y * delta_time;

            //    Logger::Log(
            //        "entity_id = " +
            //        std::to_string(entity.GetId()) +
            //        " position is now (" +
            //        std::to_string(transform.position.x) +
            //        " , " +
            //        std::to_string(transform.position.y) + ")"
            //    );

            if (entity.HasTag("player")) {
                auto& sprite = entity.GetComponent<SpriteComponent>();
                bool is_player_at_map_boundry(
                    transform.position.x <= 0 ||
                    transform.position.x + sprite.width >= Game::map_width ||
                    transform.position.y <= 0 ||
                    transform.position.y + sprite.height >= Game::map_height
                );
                if (is_player_at_map_boundry) {
                    rigid_body.velocity = glm::vec2(0, 0);
                }
                if (transform.position.x <= 0) {
                    transform.position.x = 1;
                }
                if (transform.position.x + sprite.width >= Game::map_width) {
                    transform.position.x = Game::map_width - sprite.width - 1;
                }
                if (transform.position.y <= 0) {
                    transform.position.y = 1;
                }
                if (transform.position.y + sprite.height >= Game::map_height) {
                    transform.position.y = Game::map_height - sprite.height - 1;
                }
            }

            bool is_entity_outside_map = (
                transform.position.x < 0 ||
                transform.position.x > Game::map_width ||
                transform.position.y < 0 ||
                transform.position.y > Game::map_height
                );

            // Kill all entities that move outside the map boundries.
            if (is_entity_outside_map && !entity.HasTag("player")) {
                entity.Kill();
            }
        }

    }
};