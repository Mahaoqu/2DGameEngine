#pragma once

#include <SDL.h>

#include "../ECS/ECS.h"
#include "../Components/ProjectileComponent.h"

class ProjectileLifecycleSystem : public System
{
public:
    ProjectileLifecycleSystem() {
        RequireComponent<ProjectileComponent>();
    }

    void Update() {
        for (auto entity : GetSystemEntities()) {
            const auto projectile = entity.GetComponent<ProjectileComponent>();

            // Kill projectiles after they reach their duration limit.
            if (SDL_GetTicks() - projectile.start_time > projectile.duration) {
                entity.Kill();
            }
        }
    }
};