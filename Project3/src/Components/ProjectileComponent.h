#pragma once

#include <SDL.h>

struct ProjectileComponent
{
    bool is_friendly;
    int hit_percent_damage;
    int duration;
    int start_time;

    ProjectileComponent(bool is_friendly = false, int hit_percent_damage = 0, int duration = 0) {
        this->is_friendly = is_friendly;
        this->hit_percent_damage = hit_percent_damage;
        this->duration = duration;
        this->start_time = SDL_GetTicks();
    }
};