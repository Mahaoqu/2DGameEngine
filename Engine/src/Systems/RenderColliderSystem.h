#pragma once

#include <SDL2/SDL.h>

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/BoxColliderComponent.h"

class RenderColliderSystem : public System
{
public:
    RenderColliderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxColliderComponent>();
    }

    static bool intersect(TransformComponent& a_tc, BoxColliderComponent& a_bc, TransformComponent& b_tc, BoxColliderComponent& b_bc) {
        return (
            a_tc.position.x + a_bc.offset.x <= b_tc.position.x + b_bc.offset.x + b_bc.width &&
            a_tc.position.x + a_bc.offset.x + a_bc.width >= b_tc.position.x + b_bc.offset.x &&
            a_tc.position.y + a_bc.offset.y <= b_tc.position.y + b_bc.offset.y + b_bc.height &&
            a_tc.position.y + a_bc.offset.y + a_bc.height >= b_tc.position.y + b_bc.offset.y
            );
    }

    static void draw_collision_box(SDL_Renderer* renderer, SDL_Rect& camera, TransformComponent& x_tc, BoxColliderComponent& x_bc, int r = 255, int g = 255, int b = 255, int a = 255) {
        SDL_Rect obj_rect = {
            static_cast<int>(x_tc.position.x + x_bc.offset.x - camera.x),
            static_cast<int>(x_tc.position.y + x_bc.offset.y - camera.y),
            x_bc.width * x_tc.scale.x,
            x_bc.height * x_tc.scale.y
        };
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderDrawRect(renderer, &obj_rect);
    }

    void Update(SDL_Renderer* renderer, SDL_Rect& camera) {
        auto entities = GetSystemEntities();
        for (auto i = entities.begin(); i != entities.end(); i++) {
            Entity a = *i;
            auto& a_tc = a.GetComponent<TransformComponent>();
            auto& a_bc = a.GetComponent<BoxColliderComponent>();
            for (auto j = i; j != entities.end(); j++) {
                Entity b = *j;
                if (a == b) {
                    continue;
                }
                auto& b_tc = b.GetComponent<TransformComponent>();
                auto& b_bc = b.GetComponent<BoxColliderComponent>();
                draw_collision_box(renderer, camera, a_tc, a_bc, 255, 165, 0, 255);
                draw_collision_box(renderer, camera, b_tc, b_bc, 255, 165, 0, 255);
                bool is_collision = intersect(a_tc, a_bc, b_tc, b_bc);
                if (is_collision) {
                    draw_collision_box(renderer, camera, a_tc, a_bc, 255, 0, 0, 255);
                    draw_collision_box(renderer, camera, b_tc, b_bc, 255, 0, 0, 255);
                }
            }
        }
    }
};