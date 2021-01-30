#pragma once

#include <glm\glm.hpp>
#include <SDL.h>

struct ProjectileEmitterComponent
{
    glm::vec2 projectile_velocity;
    int repeat_frequency;
    int projectile_duration;
    int hit_percent_damage;
    bool is_friendly;
    int last_emission_time;

    ProjectileEmitterComponent(
        glm::vec2 projectile_velocity = glm::vec2(0),
        int repeat_frequency = 0,
        int projectile_duration = 10000,
        int hit_percent_damage = 10,
        bool is_friendly = false
    ) {
        this->projectile_velocity = projectile_velocity;
        this->repeat_frequency = repeat_frequency;
        this->projectile_duration = projectile_duration;
        this->hit_percent_damage = hit_percent_damage;
        this->is_friendly = is_friendly;
        this->last_emission_time = SDL_GetTicks();
    }
};