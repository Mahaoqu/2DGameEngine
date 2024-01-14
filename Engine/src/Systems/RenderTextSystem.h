#pragma once

#include <SDL2/SDL.h>

#include "../AssetStore/AssetStore.h"
#include "../ECS/ECS.h"
#include "../Components/TextLabelComponent.h"

class RenderTextSystem : public System
{
public:
    RenderTextSystem() {
        RequireComponent<TextLabelComponent>();
    }

    void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& asset_store, const SDL_Rect& camera) {
        for (auto& entity : GetSystemEntities()) {
            const auto& text_label = entity.GetComponent<TextLabelComponent>();
            SDL_Surface* surface = TTF_RenderText_Blended(
                asset_store->GetFont(text_label.asset_id),
                text_label.text.c_str(),
                text_label.color
            );

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            int label_width = 0;
            int label_height = 0;

            SDL_QueryTexture(texture, NULL, NULL, &label_width, &label_height);

            SDL_Rect dest_rect = {
                static_cast<int>(text_label.position.x - (text_label.is_fixed ? 0 : camera.x)),
                static_cast<int>(text_label.position.y - (text_label.is_fixed ? 0 : camera.y)),
                label_width,
                label_height
            };

            SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        }
    }
};