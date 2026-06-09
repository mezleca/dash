#include "game.hpp"
#include "../entity/player.hpp"
#include "../physics/rigidbody.hpp"
#include "../utils/math.hpp"

#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <rlImGui.h>
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

    // load all levels data (objects will be created only when necessary)
    load_all_levels();

    while (!WindowShouldClose()) {
        if (m_current_level != nullptr) {
            UpdateMusicStream(m_current_level->music);
        }

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

void Game::load_all_levels() {
    std::cout << "[game] loading levels...\n";

    // iterate through the levels folder, then read / load all the level files
    for (const auto& entry : std::filesystem::directory_iterator(LEVELS_LOCATION)) {
        if (!entry.is_directory()) {
            continue;
        }

        for (const auto& file : std::filesystem::directory_iterator(entry)) {
            if (file.path().extension() != ".json") {
                continue;
            }

            const char* location = file.path().c_str();
            std::cout << "[game] found level at " << location << "\n";

            // create new level and load basic metadata
            DashLevel* level = new DashLevel();
            level->load(location);

            m_levels.insert({location, level});
        }
    }
}

void Game::load_level(std::string_view location, UIMode mode) {
    if (m_current_level != nullptr) {
        std::cout << "[game] what the fuck\n";
        return;
    }

    auto level_it = m_levels.find(location.data());

    if (level_it == m_levels.end()) {
        std::cout << "[game] failed to find level " << location << "\n";
        return;
    }

    DashLevel* level = level_it->second;

    // load level data if needed
    if (level->m_objects.size() == 0 && !level->m_temp_objects.empty()) {
        std::cout << "[game] loading data from level " << level->m_name << "\n";

        if (!level->load_objects()) {
            std::cout << "[game] failed to load level from " << location << "\n";
            return;
        }
    }

    // create / initialize player if needed
    if (m_player == nullptr) {
        m_player = new Player();
        m_player->position = level->m_player_start;
        std::cout << "\n player start: (" << m_player->position.x << ", " << m_player->position.y << ")\n";
    }

    // initialize raylib music, etc...
    std::filesystem::path music_full_locatino = level->m_file.parent_path() / level->m_music_file;
    level->music = LoadMusicStream(music_full_locatino.c_str());
    SetMusicPan(level->music, 0.0f);
    SetMusicVolume(level->music, 0.5f);
    PlayMusicStream(level->music);

    paused = false;
    ui.mode = mode;
    m_current_level = level;

    std::cout << "loaded " << location << " succefully" << "\n";
}

void Game::unload_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to unload current level (not found)\n";
        return;
    }

    StopMusicStream(m_current_level->music);
    m_current_level->unload();

    delete m_player;
    delete m_current_level;

    ui.mode = UIMode::MENU;
    paused = false;
}

void Game::simulate() {
    if (paused) return;
    if (ui.mode != UIMode::PLAYFIELD) return;
    if (m_player == nullptr) return;

    m_player->movement();
    m_player->rb->simulate();

    camera.target = {d_math::lerp(camera.target.x, m_player->position.x + 20.0f, 0.2f), m_player->position.y};
    camera.offset = {m_player->dimensions.x + 20.0f, m_player->dimensions.y + 40.0f};
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
