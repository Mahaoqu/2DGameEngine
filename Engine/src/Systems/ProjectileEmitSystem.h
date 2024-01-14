#pragma once

#include <SDL2/SDL.h>

#include "../ECS/ECS.h"

#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"

#include "../Components/TransformComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/CameraFollowComponent.h"

class ProjectileEmitSystem : public System
{
public:
    ProjectileEmitSystem() {
        RequireComponent<ProjectileEmitterComponent>();
        RequireComponent<TransformComponent>();
        //RequireComponent<SpriteComponent>();
        //RequireComponent<RigidBodyComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& event_bus) {
        event_bus->SubscribeToEvent<KeyPressedEvent>(this, &ProjectileEmitSystem::OnKeyPressed);
    }

    void OnKeyPressed(KeyPressedEvent& event) {
        switch (event.sdl_event.key.keysym.sym) {
            case SDLK_SPACE:
                for (auto entity : GetSystemEntities()) {
                    if (!entity.HasTag("player")) {
                        continue;
                    }
                    auto& projectile_emitter = entity.GetComponent<ProjectileEmitterComponent>();
                    const auto& transform = entity.GetComponent<TransformComponent>();

                    glm::vec2 projectile_position = transform.position;
                    if (entity.HasComponent<SpriteComponent>()) {
                        auto sprite = entity.GetComponent<SpriteComponent>();
                        projectile_position.x += (transform.scale.x * sprite.width / 2);
                        projectile_position.y += (transform.scale.y * sprite.height / 2);
                    }

                    // Add a new projectile entity to the registry.
                    Entity projectile = event.registry->CreateEntity();
                    projectile.Group("projectiles");
                    projectile.AddComponent<TransformComponent>(projectile_position, glm::vec2(1.0, 1.0), 0.0);

                    auto& rigid_body = entity.GetComponent<RigidBodyComponent>();
                    if (rigid_body.velocity.x == 0 && rigid_body.velocity.y == 0) {
                        projectile_emitter.projectile_velocity.x = 0;
                        projectile_emitter.projectile_velocity.y = -100;
                    }

                    if (rigid_body.velocity.x < 0) {
                        if (projectile_emitter.projectile_velocity.x >= 0) {
                            projectile_emitter.projectile_velocity.x = -100;
                            projectile_emitter.projectile_velocity.y = 0;
                        }
                    }
                    if (rigid_body.velocity.x > 0) {
                        if (projectile_emitter.projectile_velocity.x <= 0) {
                            projectile_emitter.projectile_velocity.x = 100;
                            projectile_emitter.projectile_velocity.y = 0;
                        }
                    }
                    if (rigid_body.velocity.y < 0) {
                        if (projectile_emitter.projectile_velocity.y >= 0) {
                            projectile_emitter.projectile_velocity.y = -100;
                            projectile_emitter.projectile_velocity.x = 0;
                        }
                    }
                    if (rigid_body.velocity.y > 0) {
                        if (projectile_emitter.projectile_velocity.y <= 0) {
                            projectile_emitter.projectile_velocity.y = 100;
                            projectile_emitter.projectile_velocity.x = 0;
                        }
                    }
                    projectile.AddComponent<RigidBodyComponent>(rigid_body.velocity + projectile_emitter.projectile_velocity);
                    projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 4);
                    projectile.AddComponent<BoxColliderComponent>(4, 4);
                    projectile.AddComponent<ProjectileComponent>(projectile_emitter.is_friendly, projectile_emitter.hit_percent_damage, projectile_emitter.projectile_duration);

                    // Update the projectile component last emission to the current milliseconds.
                    projectile_emitter.last_emission_time = SDL_GetTicks();
                    break;
                }
        }
    }

    void Update(std::unique_ptr<Registry>& registry) {
        for (auto &entity : GetSystemEntities()) {
            if (entity.HasTag("player")) {
                continue;
            }
            auto& projectile_emitter = entity.GetComponent<ProjectileEmitterComponent>();
            const auto &transform = entity.GetComponent<TransformComponent>();
            // Check if its time to re-emit a new projectile.
            if (SDL_GetTicks() - projectile_emitter.last_emission_time > projectile_emitter.repeat_frequency) {
                glm::vec2 projectile_position = transform.position;
                if (entity.HasComponent<SpriteComponent>()) {
                    auto &sprite = entity.GetComponent<SpriteComponent>();
                    projectile_position.x += (transform.scale.x * sprite.width / 2);
                    projectile_position.y += (transform.scale.y * sprite.height / 2);
                }
                // Add a new projectile entity to the registry.
                Entity projectile = registry->CreateEntity();
                projectile.Group("projectiles");
                projectile.AddComponent<TransformComponent>(projectile_position, glm::vec2(1.0, 1.0), 0.0);
                projectile.AddComponent<RigidBodyComponent>(projectile_emitter.projectile_velocity);
                projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 4);
                projectile.AddComponent<BoxColliderComponent>(4, 4);
                projectile.AddComponent<ProjectileComponent>(projectile_emitter.is_friendly, projectile_emitter.hit_percent_damage, projectile_emitter.projectile_duration);

                // Update the projectile component last emission to the current milliseconds.
                projectile_emitter.last_emission_time = SDL_GetTicks();
            }
        }
    }
};