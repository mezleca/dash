#include "game.hpp"
#include "../entity/player.hpp"
#include "../physics/rigidbody.hpp"
#include "../utils/math.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <rlImGui.h>
#include <fstream>
#include <raylib.h>
#include <string_view>

Game::Game() {
    ui.mode = UIMode::MENU;

    window.title = "dash";
    window.width = 1280;
    window.height = 720;

    camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 1.2f;
}

Game::~Game() {
}

void Game::initialize() {
    InitWindow(window.width, window.height, window.title.c_str());
    InitAudioDevice();

    SetTargetFPS(60);
    rlImGuiSetup(true);

    load_levels();

    player = new Player();
    player->visible = false;

    while (!WindowShouldClose()) {
        m_accumulator += GetFrameTime();

        while (m_accumulator >= fixed_timestep) {
            m_accumulator -= fixed_timestep;
            simulate();
        }

        alpha = m_accumulator / fixed_timestep;

        BeginDrawing();
        {
            ClearBackground({0, 0, 0, 0});
            render();
        }
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
}

void Game::load_levels() {
    std::cout << "[game] loading levels...\n";

    std::ifstream file(LEVELS_LOCATION);

    if (!file.is_open()) {
        std::cout << "[game] unable to find " << LEVELS_LOCATION << "\n";
        return;
    }

    nlohmann::json j = nlohmann::json::parse(file);
    levels = j["levels"].get<std::vector<MinimalLevelData>>();

    for (const auto& level : levels) {
        std::cout << "[game] loaded " << level.name << " data\n";
    }

    file.close();
}

void Game::save_levels() {
    std::cout << "[game] saving " << levels.size() << " levels\n";

    nlohmann::json j = {};

    for (const MinimalLevelData& level : levels) {
        j["levels"].push_back(level);
    }

    std::ofstream file(LEVELS_LOCATION);

    file << j.dump(4);
    file.close();
}

void Game::load_level(std::string_view location, UIMode mode) {
    MinimalLevelData* target_level;

    // TOFIX: stupid
    for (const auto& level : levels) {
        if (level.location == location) {
            *target_level = level;
            break;
        }
    }

    if (target_level == nullptr) {
        std::cout << "[game] failed to load " << location << "\n";
        return;
    }

    // TODO: load...

    ui.mode = mode;
    std::cout << "loaded " << location << " succefully" << "\n";
}

void Game::simulate() {
    if (paused) return;
    if (ui.mode != UIMode::PLAYFIELD) return;

    player->rb->simulate();
    player->movement();

    camera.target = {d_math::lerp(camera.target.x, player->position.x + 20.0f, 0.2f), player->position.y};

    camera.offset = {player->dimensions.x + 20.0f, player->dimensions.y + 40.0f};
}

void Game::render() {
    if (ui.mode == UIMode::MENU) {
        ui.render_main_menu();
    } else if (ui.mode == UIMode::PLAYFIELD) {
        BeginMode2D(camera);
        {
            // render all visible objects
            for (const auto& object : m_objects) {
                object->render();
            }
        }
        EndMode2D();
    }
}
