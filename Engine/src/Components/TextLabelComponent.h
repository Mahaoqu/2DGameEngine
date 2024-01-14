#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

struct TextLabelComponent
{
    glm::vec2 position;
    std::string text;
    std::string asset_id;
    SDL_Color color;
    bool is_fixed;

    TextLabelComponent(
        glm::vec2 position = glm::vec2(0),
        std::string text = "",
        std::string asset_id = "",
        const SDL_Color& color = {0, 0, 0},
        bool is_fixed = true
    ) {
        this->position = position;
        this->text = text;
        this->asset_id = asset_id;
        this->color = color;
        this->is_fixed = is_fixed;
    }
};