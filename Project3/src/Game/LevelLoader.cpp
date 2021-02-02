#include <memory>
#include <fstream>
#include <sstream>

#include <SDL.h>
#include <sol/sol.hpp>

#include "Game.h"
#include "LevelLoader.h"

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"

#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/HealthLabelComponent.h"
#include "../Components/TextLabelComponent.h"
#include "../Components/ScriptComponent.h"

LevelLoader::LevelLoader() {
    Logger::Log("LevelLoader constructor called.");
}

LevelLoader::~LevelLoader() {
    Logger::Log("LevelLoader destructor called.");
}

void LevelLoader::LoadLevel(
    sol::state& lua,
    const std::unique_ptr<Registry>& registry,
    const std::unique_ptr<AssetStore>& asset_store,
    SDL_Renderer* renderer,
    int level_num
) {

    sol::load_result script = lua.load_file("./assets/scripts/Level" + std::to_string(level_num) + ".lua");
    // Check the syntax of the script, but it does not execute the script.
    if (!script.valid()) {
        sol::error err = script;
        std::string error_message = err.what();
        Logger::Err("Error loading the Lau script: " + error_message);
        return;
    }

    // Load the entities and components from ./assets/scripts/Level1.lua.
    lua.script_file("./assets/scripts/Level" + std::to_string(level_num) + ".lua");
    Logger::Log("Opened Level" + std::to_string(level_num) + ".lua");

    // Read the big table for the current level.
    sol::table level = lua["Level"];

    // ===========================================================================
    // Read the level assets
    // ===========================================================================
    sol::table assets = level["assets"];

    int i = 0;
    while (true) {
        sol::optional<sol::table> has_asset = assets[i];
        if (has_asset == sol::nullopt) {
            break;
        }
        sol::table asset = assets[i];
        std::string asset_type = asset["type"];
        std::string asset_id = asset["id"];
        if (asset_type == "texture") {
            asset_store->AddTexture(renderer, asset_id, asset["file"]);
            Logger::Log("New texture asset loaded to the asset store, ID: " + asset_id);
        }
        if (asset_type == "font") {
            asset_store->AddFont(asset_id, asset["file"], asset["font_size"]);
            Logger::Log("New font asset loaded to the asset store, ID: " + asset_id);
        }
        i++;
    }

    // ===========================================================================
    // Create tilemap for the level.
    // ===========================================================================

    sol::table tilemap = level["tilemap"];
    std::string tilemap_map_file = tilemap["map_file"];
    std::string tilemap_asset_id = tilemap["texture_asset_id"];
    int tilemap_num_rows = tilemap["num_rows"];
    int tilemap_num_cols = tilemap["num_cols"];
    int tilemap_tile_size = tilemap["tile_size"];
    int tilemap_scale = tilemap["scale"];

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

    std::ifstream file(tilemap_map_file);
    std::string str_line;
    std::vector<std::tuple<size_t, size_t, size_t>> tilemap_vec;
    size_t map_ycoord = 0;
    while (std::getline(file, str_line)) {
        std::stringstream sub_stream(str_line);
        size_t map_xcoord = 0;
        while (sub_stream.good()) {
            std::string substr;
            std::getline(sub_stream, substr, ',');
            tilemap_vec.push_back(std::make_tuple(static_cast<size_t>(std::stoi(substr)), map_xcoord, map_ycoord));
            map_xcoord += tilemap_tile_size;
        }
        map_ycoord += tilemap_tile_size;
    }
    file.close();

    Game::map_width = tilemap_num_cols * tilemap_tile_size * tilemap_scale;
    Game::map_height = tilemap_num_rows * tilemap_tile_size * tilemap_scale;

    std::vector<Entity> tiles;
    for (auto& tilemap_tile : tilemap_vec) {
        Entity tile = registry->CreateEntity();
        tile.Group("tiles");
        tile.AddComponent<TransformComponent>(glm::vec2(std::get<1>(tilemap_tile) * tilemap_scale, std::get<2>(tilemap_tile) * tilemap_scale), glm::vec2(tilemap_scale, tilemap_scale), 0.0);
        tile.AddComponent<SpriteComponent>(tilemap_asset_id, tilemap_tile_size, tilemap_tile_size, 0, false, std::get<0>(tile_srcs[std::get<0>(tilemap_tile)]), std::get<1>(tile_srcs[std::get<0>(tilemap_tile)]));
    }


    // ===========================================================================
    // Create entities
    // ===========================================================================
    sol::table entities = level["entities"];

    int j = 0;
    while (true) {
        sol::optional<sol::table> has_entity = entities[j];
        if (has_entity == sol::nullopt) {
            break;
        }
        sol::table entity = entities[j];

        Entity new_entity = registry->CreateEntity();

        // Tag
        sol::optional<std::string> tag = entity["tag"];
        if (tag != sol::nullopt) {
            new_entity.Tag(entity["tag"]);
        }

        // Group
        sol::optional<std::string> group = entity["group"];
        if (group != sol::nullopt) {
            new_entity.Group(entity["group"]);
        }

        // Components
        sol::optional<sol::table> has_components = entity["components"];
        if (has_components != sol::nullopt) {
            // Transform
            sol::optional<sol::table> transform = entity["components"]["transform"];
            if (transform != sol::nullopt) {
                new_entity.AddComponent<TransformComponent>(
                    glm::vec2(
                        entity["components"]["transform"]["position"]["x"],
                        entity["components"]["transform"]["position"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["transform"]["scale"]["x"].get_or(1.0),
                        entity["components"]["transform"]["scale"]["y"].get_or(1.0)
                    ),
                    entity["components"]["transform"]["rotation"].get_or(0.0)
                    );
            }

            // RigidBody
            sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
            if (rigidbody != sol::nullopt) {
                new_entity.AddComponent<RigidBodyComponent>(
                    glm::vec2(
                        entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
                        entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
                    )
                    );
            }

            // Sprite
            sol::optional<sol::table> sprite = entity["components"]["sprite"];
            if (sprite != sol::nullopt) {
                new_entity.AddComponent<SpriteComponent>(
                    entity["components"]["sprite"]["texture_asset_id"],
                    entity["components"]["sprite"]["width"],
                    entity["components"]["sprite"]["height"],
                    entity["components"]["sprite"]["z_index"].get_or(1),
                    entity["components"]["sprite"]["fixed"].get_or(false),
                    entity["components"]["sprite"]["src_rect_x"].get_or(0),
                    entity["components"]["sprite"]["src_rect_y"].get_or(0)
                    );
            }

            // Animation
            sol::optional<sol::table> animation = entity["components"]["animation"];
            if (animation != sol::nullopt) {
                new_entity.AddComponent<AnimationComponent>(
                    entity["components"]["animation"]["num_frames"].get_or(1),
                    entity["components"]["animation"]["speed_rate"].get_or(1)
                    );
            }

            // BoxCollider
            sol::optional<sol::table> collider = entity["components"]["boxcollider"];
            if (collider != sol::nullopt) {
                new_entity.AddComponent<BoxColliderComponent>(
                    entity["components"]["boxcollider"]["width"],
                    entity["components"]["boxcollider"]["height"],
                    glm::vec2(
                        entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
                        entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
                    )
                    );
            }

            // Health
            sol::optional<sol::table> health = entity["components"]["health"];
            if (health != sol::nullopt) {
                new_entity.AddComponent<HealthComponent>(
                    static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
                    );
            }

            // HealthLabel
            sol::optional<sol::table> health_label = entity["components"]["health_label"];
            if (health_label != sol::nullopt) {
                sol::table font_color = entity["components"]["health_label"]["color"];
                int font_color_r = static_cast<int>(font_color["r"].get_or(0));
                int font_color_g = static_cast<int>(font_color["g"].get_or(0));
                int font_color_b = static_cast<int>(font_color["b"].get_or(0));
                SDL_Color font_color_rgb = {font_color_r, font_color_g, font_color_b};
                new_entity.AddComponent<HealthLabelComponent>(entity["components"]["health_label"]["font"], font_color_rgb);
            }

            // ProjectileEmitter
            sol::optional<sol::table> projectile_emitter = entity["components"]["projectile_emitter"];
            if (projectile_emitter != sol::nullopt) {
                new_entity.AddComponent<ProjectileEmitterComponent>(
                    glm::vec2(
                        entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
                        entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
                    ),
                    static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
                    static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
                    static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
                    entity["components"]["projectile_emitter"]["friendly"].get_or(false)
                    );
            }

            // CameraFollow
            sol::optional<sol::table> camera_follow = entity["components"]["camera_follow"];
            if (camera_follow != sol::nullopt) {
                new_entity.AddComponent<CameraFollowComponent>();
            }

            // KeyboardControlled
            sol::optional<sol::table> keyboard_controller = entity["components"]["keyboard_controller"];
            if (keyboard_controller != sol::nullopt) {
                new_entity.AddComponent<KeyboardControlledComponent>(
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["up_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["up_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["right_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["right_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["down_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["down_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["left_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["left_velocity"]["y"]
                    )
                    );
            }

            // Script
            sol::optional<sol::table> script = entity["components"]["on_update_script"];
            if (script != sol::nullopt) {
                sol::function func = entity["components"]["on_update_script"][0];
                new_entity.AddComponent<ScriptComponent>(func);
            }
        }
        j++;
    }


    //Entity label = registry->CreateEntity();
    //label.AddComponent<TextLabelComponent>(glm::vec2(Game::window_width / 2 - 40, 10), "CHOPPER 1.0", "charriot-font", green, true);
}
