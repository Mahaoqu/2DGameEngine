#pragma once

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"

const int FPS = 60;
const int MILLISECONDS_PER_FRAME = 1000 / FPS;

class Game
{
private:
    bool is_running;
    bool is_debug;
    int milliseconds_previous_frame = 0;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Rect camera;

    std::unique_ptr<Registry> registry;
    std::unique_ptr<AssetStore> asset_store;
    std::unique_ptr<EventBus> event_bus;

public:
    Game();
    ~Game();
    void Initialize();
    void Run();
    void ProcessInput();
    void LoadLevel(int level);
    void Setup();
    void Update();
    void Render();
    void Destroy();

    static int window_width;
    static int window_height;
    static int map_width;
    static int map_height;
};