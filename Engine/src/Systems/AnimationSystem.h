#pragma once

#include "../ECS/ECS.h"
#include "../Components/AnimationComponent.h"
#include "../Components/SpriteComponent.h"

class AnimationSystem : public System
{
public:
    AnimationSystem() {
        RequireComponent<SpriteComponent>();
        RequireComponent<AnimationComponent>();
    }

    void Update() {
        for (auto entity : GetSystemEntities()) {
            auto& animation = entity.GetComponent<AnimationComponent>();
            auto& sprite = entity.GetComponent<SpriteComponent>();

            animation.current_frame = fmod(((SDL_GetTicks() - (size_t) animation.start_time) * (animation.frame_speed_rate / 1000.0)), animation.num_frames);
            sprite.src_rect.x = animation.current_frame * sprite.width;

            // TODO:
            // change the current frame.
            // change the src rectangle of the sprite.
            // ...
        }
    }
};
