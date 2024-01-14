#pragma once

#include <SDL2/SDL.h>
#include <sol/sol.hpp>
#include <memory>

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"

class LevelLoader
{
public:
    LevelLoader();
    ~LevelLoader();
    void LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, const std::unique_ptr<AssetStore>& asset_store, SDL_Renderer* renderer, int level);
};