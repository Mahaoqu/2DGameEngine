#include "Game.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>

#include "LevelLoader.h"

#include "../Logger/Logger.h"

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"

#include "../Components/AnimationComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
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
#include "../Systems/RenderGUISystem.h"
#include "../Systems/ScriptSystem.h"


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
    window_width = 1920;
    window_height = 1080;
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
        
        // ImGui SDL input
        ImGui_ImplSDL2_ProcessEvent(&sdl_event);
        ImGuiIO& io = ImGui::GetIO();
        int mouse_x, mouse_y;
        const int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        io.MousePos = ImVec2(mouse_x, mouse_y);
        io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        // Handle core SDL events (close window, key pressed, etc.)
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

void Game::Setup() {
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
    registry->AddSystem<RenderGUISystem>();
    registry->AddSystem<ScriptSystem>();

    // Create bindings between C++ and Lua.
    registry->GetSystem<ScriptSystem>().CreateLuaBindings(lua);

    LevelLoader loader;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
    loader.LoadLevel(lua, registry, asset_store, renderer, 1);
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
    registry->GetSystem<MovementSystem>().SubscribeToEvents(event_bus);
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
    registry->GetSystem<ScriptSystem>().Update(delta_time, SDL_GetTicks());
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
        registry->GetSystem<RenderGUISystem>().Update(registry, asset_store);

        // Show the ImGui demo window.
        //ImGui::NewFrame();
        //ImGui::ShowDemoWindow();
        //ImGui::Render();
        //ImGuiSDL::Render(ImGui::GetDrawData());
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
