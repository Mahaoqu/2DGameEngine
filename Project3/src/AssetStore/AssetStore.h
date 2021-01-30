#pragma once

#include <map>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class AssetStore
{
private:
    std::map<std::string, SDL_Texture*> textures;
    std::map<std::string, TTF_Font*> fonts;
    // TODO: create a map for audio
public:
    AssetStore();
    ~AssetStore();

    void ClearAssets();
    void AddTexture(SDL_Renderer* renderer, const std::string& asset_id, const std::string& file_path);
    SDL_Texture* GetTexture(const std::string& asset_id);

    void AddFont(const std::string& asset_id, const std::string& file_path, int font_size);
    TTF_Font* GetFont(const std::string& asset_id);
};