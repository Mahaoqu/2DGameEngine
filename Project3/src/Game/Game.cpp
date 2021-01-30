#include "Game.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>

#include "../Logger/Logger.h"

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TextLabelComponent.h"

#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"

int Game::window_width;
int Game::window_height;
int Game::map_width;
int Game::map_height;

Game::Game() {
    is_running = false;
    is_debug = false;
    registry = std::make_unique<Registry>();
    asset_store = std::make_unique<AssetStore>();
    event_bus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called.");
}

Game::~Game() {
    Logger::Log("Game destructor called.");
    //for (auto it = begin(Logger::messages); it != end(Logger::messages); ++it) {
    //    std::cout << it->message << std::endl;
    //}
}

void Game::Initialize() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Err("Error initializing SDL.");
        return;
    }

    if (TTF_Init() != 0) {
        Logger::Err("Error initializing SDL TTF.");
        return;
    }
    // ==========================================================================
    // Get the (fake) full screen display width and height.
    // ==========================================================================
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    //window_width = display_mode.w;
    //window_height = display_mode.h;
    window_width = 800;
    window_height = 600;
    // ==========================================================================
    // Create the window.
    // ==========================================================================
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        Logger::Err("Error creating SDL window.");
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        Logger::Err("Error creating SDL renderer.");
        return;
    }
    // ==========================================================================
    // This will change the video mode of the display to be a real fullscreen.
    // ==========================================================================
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    // Initialize the ImGui context.
    ImGui::CreateContext();
    ImGuiSDL::Initialize(renderer, window_width, window_height);

    // Initialize the camera view with the entire screen area.
    camera.x = 0;
    camera.y = 0;
    camera.w = window_width;
    camera.h = window_height;

    is_running = true;
}

void Game::Run() {
    Setup();
    while (is_running) {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::ProcessInput() {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
            case SDL_KEYDOWN:
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                    is_running = false;
                }
                if (sdl_event.key.keysym.sym == SDLK_d) {
                    is_debug = !is_debug;
                }
                event_bus->EmitEvent<KeyPressedEvent>(sdl_event, registry);
                break;
        }
    }
}

void Game::LoadLevel(int level) {
    // Add the systems that need to be processed in our game.
    registry->AddSystem<MovementSystem>();
    registry->AddSystem<RenderSystem>();
    registry->AddSystem<AnimationSystem>();
    registry->AddSystem<CollisionSystem>();
    registry->AddSystem<RenderColliderSystem>();
    registry->AddSystem<DamageSystem>();
    registry->AddSystem<KeyboardControlSystem>();
    registry->AddSystem<CameraMovementSystem>();
    registry->AddSystem<ProjectileEmitSystem>();
    registry->AddSystem<ProjectileLifecycleSystem>();
    registry->AddSystem<RenderTextSystem>();
    registry->AddSystem<RenderHealthTextSystem>();
    registry->AddSystem<RenderHealthBarSystem>();

    // Add assets to the asset_store.
    asset_store->AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
    asset_store->AddTexture(renderer, "truck-image", "./assets/images/truck-ford-right.png");
    asset_store->AddTexture(renderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
    asset_store->AddTexture(renderer, "radar-image", "./assets/images/radar.png");
    asset_store->AddTexture(renderer, "jungle", "./assets/tilemaps/jungle.png");
    asset_store->AddTexture(renderer, "bullet-image", "./assets/images/bullet.png");
    asset_store->AddFont("charriot-font", "./assets/fonts/charriot.ttf", 20);

    std::vector<std::tuple<size_t, size_t>> tile_srcs;
    size_t ycoord = 0;
    for (size_t i = 0; i < 3; i++) {
        size_t xcoord = 0;
        for (size_t j = 0; j < 10; j++) {
            tile_srcs.push_back(std::make_tuple(xcoord, ycoord));
            xcoord += 32;
        }
        ycoord += 32;
    }

    //for (auto tile_src : tile_srcs) {
    //    Logger::Log("(" + std::to_string(std::get<0>(tile_src)) + ", " + std::to_string(std::get<1>(tile_src)) + ")");
    //}

    std::ifstream file("./assets/tilemaps/jungle.map");
    std::string str_line;
    std::vector<std::tuple<size_t, size_t, size_t>> jungle_map;
    size_t map_ycoord = 0;
    while (std::getline(file, str_line)) {
        std::stringstream sub_stream(str_line);
        size_t map_xcoord = 0;
        while (sub_stream.good()) {
            std::string substr;
            std::getline(sub_stream, substr, ',');
            jungle_map.push_back(std::make_tuple(static_cast<size_t>(std::stoi(substr)), map_xcoord, map_ycoord));
            map_xcoord += 32;
        }
        map_ycoord += 32;
    }
    file.close();

    map_width = 25 * 32 * 2;
    map_height = 20 * 32 * 2;

    size_t tile_scale = 2.0;

    std::vector<Entity> tiles;
    for (auto jungle_tile : jungle_map) {
        //Logger::Log(
        //    "(" + std::to_string(std::get<0>(jungle_tile)) + ", " +
        //    std::to_string(std::get<1>(jungle_tile)) + ", " +
        //    std::to_string(std::get<2>(jungle_tile)) + ")"
        //);
        Entity tile = registry->CreateEntity();
        tile.Group("tiles");
        tile.AddComponent<TransformComponent>(glm::vec2(std::get<1>(jungle_tile) * tile_scale, std::get<2>(jungle_tile) * tile_scale), glm::vec2(tile_scale, tile_scale), 0.0);
        tile.AddComponent<SpriteComponent>("jungle", 32, 32, 0, false, std::get<0>(tile_srcs[std::get<0>(jungle_tile)]), std::get<1>(tile_srcs[std::get<0>(jungle_tile)]));
    }

    SDL_Color green = {0, 255, 0};

    // Create entities.
    Entity chopper = registry->CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<TransformComponent>(glm::vec2(10.0, 100.0), glm::vec2(1.0, 1.0), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 1);
    chopper.AddComponent<BoxColliderComponent>(32, 32);
    chopper.AddComponent<AnimationComponent>(2, 15, true);
    chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0, -150), glm::vec2(150, 0), glm::vec2(0, 150), glm::vec2(-150, 0));
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, 0.0), 0, 3000, 10, true);
    chopper.AddComponent<HealthComponent>(100);
    chopper.AddComponent<HealthLabelComponent>("charriot-font", green);

    Entity radar = registry->CreateEntity();
    radar.AddComponent<TransformComponent>(glm::vec2(window_width * 2 - 74, 10.0), glm::vec2(1.0, 1.0), 0.0);
    radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 1, true);
    radar.AddComponent<AnimationComponent>(8, 5, true);

    Entity tank = registry->CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(glm::vec2(300.00, 500.0), glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
    tank.AddComponent<BoxColliderComponent>(32, 32);
    tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 5000, 3000, 10, false);
    tank.AddComponent<HealthComponent>(100);
    tank.AddComponent<HealthLabelComponent>("charriot-font", green);

    Entity truck = registry->CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(glm::vec2(150.0, 500.0), glm::vec2(1.0, 1.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 1);
    truck.AddComponent<BoxColliderComponent>(32, 32);
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, 100.0), 2000, 5000, 10, false);
    truck.AddComponent<HealthComponent>(100);
    truck.AddComponent<HealthLabelComponent>("charriot-font", green);

    Entity label = registry->CreateEntity();
    label.AddComponent<TextLabelComponent>(glm::vec2(window_width / 2 - 40, 10), "CHOPPER 1.0", "charriot-font", green, true);
}

void Game::Setup() {
    LoadLevel(1);
}

void Game::Update() {

    // If we are too fast, waste some time until we reach the MILLISECONDS_PER_FRAME
    //while (!SDL_TICKS_PASSED(SDL_GetTicks(), milliseconds_previous_frame + MILLISECONDS_PER_FRAME));
    int time_to_wait = MILLISECONDS_PER_FRAME - (SDL_GetTicks() - milliseconds_previous_frame);

    if (time_to_wait > 0 && time_to_wait <= MILLISECONDS_PER_FRAME) {
        SDL_Delay(time_to_wait);
    }

    double delta_time = (static_cast<double>(SDL_GetTicks()) - static_cast<double>(milliseconds_previous_frame)) / 1000.0;

    // Store the current frame time in milliseconds.
    milliseconds_previous_frame = SDL_GetTicks();

    // Reset all event handlers for the current frame.
    event_bus->Reset();

    // Perform subscription of the events for all systems.
    registry->GetSystem<DamageSystem>().SubscribeToEvents(event_bus);
    registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(event_bus);
    registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(event_bus);

    // Update the registry to process the entities that are waiting to be created or deleted.
    registry->Update();

    // Invoke all the systems that need to update.
    registry->GetSystem<MovementSystem>().Update(delta_time);
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(event_bus);
    registry->GetSystem<ProjectileEmitSystem>().Update(registry);
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectileLifecycleSystem>().Update();
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    // Render all the game objects.
    //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //SDL_Rect player = {10, 10, 20, 20};
    //SDL_RenderFillRect(renderer, &player);

    // Draw a PNG texture
    //SDL_Surface* surface = IMG_Load("./assets/images/tank-tiger-right.png");
    //SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    //SDL_FreeSurface(surface);

    // What is the destination rectangle that we want to place our texture.
    //SDL_Rect dst_rect = {
    //  static_cast<int>(player_position.x),
    //  static_cast<int>(player_position.y),
    //  32,
    //  32
    //};

    //SDL_RenderCopy(renderer, texture, NULL, &dst_rect);

    //SDL_DestroyTexture(texture);

    registry->GetSystem<RenderSystem>().Update(renderer, asset_store, camera);
    registry->GetSystem<RenderTextSystem>().Update(renderer, asset_store, camera);
    registry->GetSystem<RenderHealthTextSystem>().Update(renderer, asset_store, camera);
    registry->GetSystem<RenderHealthBarSystem>().Update(renderer, camera);
    if (is_debug) {
        registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
    }
    SDL_RenderPresent(renderer);
}

void Game::Destroy() {
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
