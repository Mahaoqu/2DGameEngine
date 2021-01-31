#pragma once

#include <SDL.h>
#include <algorithm>

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"

class RenderSystem : public System
{
public:
    RenderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    static bool compare_by_zindex(const Entity& a, const Entity& b) {
        int a_zindex = a.GetComponent<SpriteComponent>().zindex;
        int b_zindex = b.GetComponent<SpriteComponent>().zindex;
        //Logger::Log("a_zindex = " + std::to_string(a_zindex));
        //Logger::Log("b_zindex = " + std::to_string(b_zindex));
        return a_zindex < b_zindex;
    }

    void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& asset_store, SDL_Rect& camera) {

        auto entities = GetSystemEntities();
        std::sort(entities.begin(), entities.end(), compare_by_zindex);

        // Loop all entities that the system is interested in...
        for (auto entity : entities) {
            // Update entity position based on its velocity.
            const auto& transform = entity.GetComponent<TransformComponent>();
            const auto sprite = entity.GetComponent<SpriteComponent>();

            bool is_entity_outside_camera_view = (
                transform.position.x + (transform.scale.x * sprite.width)  < camera.x ||
                transform.position.x > camera.x + camera.w ||
                transform.position.y + (transform.scale.y * sprite.height) < camera.y ||
                transform.position.y > camera.y + camera.h
            );

            // Cull sprites that are outside the camera view and are not fixed.
            if (is_entity_outside_camera_view && !sprite.is_fixed) {
                continue;
            }

            // Set the source rectangle of our original sprite texture.
            SDL_Rect src_rect = sprite.src_rect;

            // Set the destination rectangle with the xy position to be rendered.
            SDL_Rect dst_rect = {
                static_cast<int>(transform.position.x - (sprite.is_fixed ? 0 : camera.x)),
                static_cast<int>(transform.position.y - (sprite.is_fixed ? 0 : camera.y)),
                static_cast<int>(sprite.width * transform.scale.x),
                static_cast<int>(sprite.height * transform.scale.y)
            };

            SDL_RenderCopyEx(
                renderer,
                asset_store->GetTexture(sprite.asset_id),
                &src_rect,
                &dst_rect,
                transform.rotation,
                NULL,
                sprite.flip
            );

            //SDL_Rect obj_rect = {
            //    static_cast<int>(transform.position.x),
            //    static_cast<int>(transform.position.y),
            //    sprite.width,
            //    sprite.height
            //};
            //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            //SDL_RenderFillRect(renderer, &obj_rect);
        }

    }
};