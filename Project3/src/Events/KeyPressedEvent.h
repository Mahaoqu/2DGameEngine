#pragma once

#include "../ECS/ECS.h"
#include "../EventBus/Event.h"

class KeyPressedEvent : public Event
{
public:
    SDL_Event sdl_event;
    std::unique_ptr<Registry>& registry;
    KeyPressedEvent(SDL_Event sdl_event, std::unique_ptr<Registry>& registry) : sdl_event(sdl_event), registry(registry) {}
};