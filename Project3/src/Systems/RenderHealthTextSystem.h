#pragma once

#include <SDL.h>
#include <string>

#include "../ECS/ECS.h"

#include "../AssetStore/AssetStore.h"

#include "../Components/HealthLabelComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TransformComponent.h"

class RenderHealthTextSystem : public System
{
public:
    RenderHealthTextSystem() {
        RequireComponent<HealthLabelComponent>();
        RequireComponent<HealthComponent>();
        RequireComponent<TransformComponent>();
    }

    void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& asset_store, const SDL_Rect& camera) {
        for (auto& entity : GetSystemEntities()) {
            if (entity.HasTag("player") || entity.BelongsToGroup("enemies")) {
                auto& health = entity.GetComponent<HealthComponent>();
                auto& transform = entity.GetComponent<TransformComponent>();

                auto& health_label = entity.GetComponent<HealthLabelComponent>();
                health_label.text = std::to_string(health.health_percentage) + "%";

                health_label.position.x = transform.position.x + 20;
                health_label.position.y = transform.position.y - 25;

                if (health.health_percentage >= 71) {
                    health_label.color = {0, 255, 0};
                }

                if (health.health_percentage >= 1 && health.health_percentage <= 70) {
                    health_label.color = {255, 165, 0};
                }

                if (health.health_percentage >= 0 && health.health_percentage <= 20) {
                    health_label.color = {255, 0, 0};
                }

                SDL_Surface* surface = TTF_RenderText_Blended(
                    asset_store->GetFont(health_label.asset_id),
                    health_label.text.c_str(),
                    health_label.color
                );

                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);

                int label_width = 0;
                int label_height = 0;

                SDL_QueryTexture(texture, NULL, NULL, &label_width, &label_height);

                SDL_Rect dest_rect = {
                    static_cast<int>(health_label.position.x - camera.x),
                    static_cast<int>(health_label.position.y - camera.y),
                    label_width,
                    label_height
                };

                SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
            }
        }
    }
};