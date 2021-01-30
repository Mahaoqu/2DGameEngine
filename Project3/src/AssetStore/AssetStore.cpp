#include "AssetStore.h"
#include "../Logger/Logger.h"

#include <SDL_image.h>

AssetStore::AssetStore() {
    Logger::Log("AssetStore constructor called.");
}

AssetStore::~AssetStore() {
    ClearAssets();
    Logger::Log("AssetStore destructor called.");
}

void AssetStore::ClearAssets() {
    for (std::pair<std::string, SDL_Texture*> texture : textures) {
        SDL_DestroyTexture(texture.second);
    }
    textures.clear();
    for (auto& font : fonts) {
        TTF_CloseFont(font.second);
    }
    fonts.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& asset_id, const std::string& file_path) {
    SDL_Surface* surface = IMG_Load(file_path.c_str());
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // Add the texture to the map.
    textures.emplace(asset_id, texture);

    //Logger::Log("New texture added to the AssetStore with asset_id = " + asset_id);

}

SDL_Texture* AssetStore::GetTexture(const std::string& asset_id) {
    return textures[asset_id];
}

void AssetStore::AddFont(const std::string& asset_id, const std::string& file_path, int font_size) {
    fonts.emplace(asset_id, TTF_OpenFont(file_path.c_str(), font_size));
}

TTF_Font* AssetStore::GetFont(const std::string& asset_id) {
    return fonts[asset_id];
}