#pragma once

#include <math.h>

#include "../ECS/ECS.h"
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <glm/glm.hpp>

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"

class RenderGUISystem : public System
{
public:
    RenderGUISystem() = default;

    void Update(const std::unique_ptr<Registry>& registry, std::unique_ptr<AssetStore>& asset_store) {
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::Begin("Spawn enemies", NULL, window_flags)) {
            // =============================================================
            // Enemy starting position.
            // =============================================================
            static int x_pos = 500, y_pos = 500;
            ImGui::InputInt("x-position", &x_pos);
            ImGui::InputInt("y-position", &y_pos);
            // =============================================================
            // Enemy scale.
            // =============================================================
            static int x_scale = 1, y_scale = 1;
            ImGui::InputInt("x-scale", &x_scale);
            ImGui::InputInt("y-scale", &y_scale);
            // =============================================================
            // Enemy rotation.
            // =============================================================
            static int rotation = 90;
            ImGui::InputInt("rotation", &rotation);
            // =============================================================
            // Enemy velocity.
            // =============================================================
            static int x_vel = 0, y_vel = 0;
            ImGui::InputInt("x-velocity", &x_vel);
            ImGui::InputInt("y-velocity", &y_vel);
            // =============================================================
            // Sprite texture ID.
            // =============================================================
            const char* items[] = {"tank-image", "truck-image"};
            static int item_current = 0;
            ImGui::Combo("select sprite", &item_current, items, IM_ARRAYSIZE(items));
            std::string sprite_image(items[item_current]);
            // =============================================================
            // Angle and speed of projects being emitted.
            // =============================================================
            static int px_vel = 100, py_vel = 0;
            ImGui::InputInt("px-velocity", &px_vel);
            ImGui::InputInt("py-velocity", &py_vel);
            // =============================================================
            // Repeat frequency and duration of projects (in seconds).
            // =============================================================
            static int freq = 5, dur = 3;
            ImGui::InputInt("frequency", &freq);
            ImGui::InputInt("duration", &dur);
            // =============================================================
            // Initial hit percentage of the enemy.
            // =============================================================
            static int hitp = 100;
            ImGui::InputInt("hit-percent", &hitp);

            if (ImGui::Button("create")) {
                Entity enemy = registry->CreateEntity();
                enemy.Group("enemies");
                enemy.AddComponent<TransformComponent>(glm::vec2(x_pos, y_pos), glm::vec2(x_scale, y_scale), rotation * (M_PI / 180));
                enemy.AddComponent<RigidBodyComponent>(glm::vec2(x_vel, y_vel));
                enemy.AddComponent<SpriteComponent>(sprite_image, 32, 32, 1);
                enemy.AddComponent<BoxColliderComponent>(32, 32);
                enemy.AddComponent<ProjectileEmitterComponent>(glm::vec2(px_vel, py_vel), freq * 1000, dur * 1000, 10, false);
                enemy.AddComponent<HealthComponent>(hitp);
                SDL_Color green = {0, 255, 0};
                enemy.AddComponent<HealthLabelComponent>("charriot-font", green);
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
    }
};