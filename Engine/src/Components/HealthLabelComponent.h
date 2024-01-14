#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

struct HealthLabelComponent
{
    glm::vec2 position;
    std::string text;
    std::string asset_id;
    SDL_Color color;

    HealthLabelComponent(std::string asset_id = "", const SDL_Color& color = {0, 0, 0}) {
        this->position = glm::vec2(0);
        this->text = "";
        this->asset_id = asset_id;
        this->color = color;
    }
};
