#pragma once

#include <string>
#include <SDL.h>

struct SpriteComponent
{
    std::string asset_id;
    int width;
    int height;
    int zindex;
    bool is_fixed;
    SDL_Rect src_rect;

    SpriteComponent(
        std::string asset_id = "",
        int width = 0,
        int height = 0,
        int zindex = 0,
        bool is_fixed = false,
        int src_rect_x = 0,
        int src_rect_y = 0
    ) {
        this->asset_id = asset_id;
        this->width = width;
        this->height = height;
        this->zindex = zindex;
        this->is_fixed = is_fixed;
        this->src_rect = {src_rect_x, src_rect_y, width, height};
    }
};