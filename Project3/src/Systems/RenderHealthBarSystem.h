#pragma once

#include <SDL.h>
#include <string>

#include "../ECS/ECS.h"

#include "../AssetStore/AssetStore.h"

#include "../Components/HealthLabelComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TransformComponent.h"

class RenderHealthBarSystem : public System
{
public:
    RenderHealthBarSystem() {
        RequireComponent<HealthLabelComponent>();
        RequireComponent<HealthComponent>();
        RequireComponent<TransformComponent>();
    }

    void Update(SDL_Renderer* renderer, const SDL_Rect& camera) {
        for (auto& entity : GetSystemEntities()) {
            if (entity.HasTag("player") || entity.BelongsToGroup("enemies")) {
                auto& health = entity.GetComponent<HealthComponent>();
                auto& transform = entity.GetComponent<TransformComponent>();

                auto& health_label = entity.GetComponent<HealthLabelComponent>();

                health_label.position.x = transform.position.x + 20;
                health_label.position.y = transform.position.y - 3;

                if (health.health_percentage >= 71) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                }

                if (health.health_percentage >= 1 && health.health_percentage <= 70) {
                    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
                }

                if (health.health_percentage >= 0 && health.health_percentage <= 20) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }

                SDL_Rect dest_rect = {
                    static_cast<int>(health_label.position.x - camera.x),
                    static_cast<int>(health_label.position.y - camera.y),
                    static_cast<int>(health.health_percentage / 2),
                    5
                };

                SDL_RenderFillRect(renderer, &dest_rect);
                SDL_RenderCopy(renderer, NULL, NULL, &dest_rect);

            }
        }
    }
};