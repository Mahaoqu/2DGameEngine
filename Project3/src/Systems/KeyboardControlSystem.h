#pragma once

#include "../ECS/ECS.h"

#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"

#include "../Components/KeyboardControlledComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"

class KeyboardControlSystem : public System
{
public:
    KeyboardControlSystem() {
        RequireComponent<KeyboardControlledComponent>();
        RequireComponent<SpriteComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& event_bus) {
        event_bus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::OnKeyPressed);
    }

    void OnKeyPressed(KeyPressedEvent& event) {
        //Logger::Log("Key pressed: " + static_cast<std::string>(SDL_GetKeyName(event.sdl_event.key.keysym.sym)));
        // Change the sprite and the velocity of my entity.
        for (auto entity : GetSystemEntities()) {
            const auto keyboard_control = entity.GetComponent<KeyboardControlledComponent>();
            auto& sprite = entity.GetComponent<SpriteComponent>();
            auto& rigid_body = entity.GetComponent<RigidBodyComponent>();

            switch (event.sdl_event.key.keysym.sym) {
                case SDLK_UP:
                    rigid_body.velocity = keyboard_control.up_velocity;
                    sprite.src_rect.y = sprite.height * 0;
                    break;
                case SDLK_RIGHT:
                    rigid_body.velocity = keyboard_control.right_velocity;
                    sprite.src_rect.y = sprite.height * 1;
                    break;
                case SDLK_DOWN:
                    rigid_body.velocity = keyboard_control.down_velocity;
                    sprite.src_rect.y = sprite.height * 2;
                    break;
                case SDLK_LEFT:
                    rigid_body.velocity = keyboard_control.left_velocity;
                    sprite.src_rect.y = sprite.height * 3;
                    break;
            }
        }
    }

    void Update() {

    }
};