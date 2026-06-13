#include "game.hpp"
#include "../entity/player.hpp"
#include "../physics/rigidbody.hpp"
#include "../utils/math.hpp"
#include "object.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <rlImGui.h>
#include <raylib.h>
#include <string_view>

static constexpr float CAMERA_PLATFORM_X_THRESHOLD = 200.0f;
static constexpr float CAMERA_PLATFORM_Y_THRESHOLD = 1000.0f;
static constexpr float CAMERA_Y_SMOOTHING = 0.09f;
static constexpr float CAMERA_X_LOOK_AHEAD = 128.0f;
static constexpr float CAMERA_Y_LOOK_AHEAD = -128.0f;

Game::Game() {
    m_ui.mode = UIMode::MENU;

    m_window.title = "dash";
    m_window.width = 1280;
    m_window.height = 720;

    m_camera = {0};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.2f;
}

Game::~Game() {
}

void Game::initialize() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());
    InitAudioDevice();

    SetTargetFPS(60);
    SetExitKey(0);

    rlImGuiSetup(true);

    // load all levels data (objects will be created only when necessary)
    load_all_levels();

    // load ui textures, etc...
    m_ui.initialize();

    while (!WindowShouldClose()) {
        handle_pause_state();
        update_simulation_timestep();

        // handle resize
        if (IsWindowResized()) {
            m_window.width = GetScreenWidth();
            m_window.height = GetScreenHeight();
        }

        update_current_level_progress();

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

void Game::update_simulation_timestep() {
    if (m_paused) {
        m_accumulator = 0.0f;
        m_alpha = 0.0f;
        return;
    }

    m_accumulator += GetFrameTime();

    while (m_accumulator >= m_fixed_frametime) {
        m_accumulator -= m_fixed_frametime;
        simulate();
    }

    m_alpha = m_accumulator / m_fixed_frametime;
}

void Game::handle_pause_state() {
    if (m_paused == m_was_paused) {
        return;
    }

    if (m_paused) {
        m_accumulator = 0.0f;
        m_alpha = 0.0f;
        pause_current_level_music();
    } else {
        resume_current_level_music();
    }

    m_was_paused = m_paused;
}

void Game::pause_current_level_music() {
    if (m_current_level == nullptr) {
        return;
    }

    std::cout << "[game] pausing music\n";
    PauseMusicStream(m_current_level->music);
}

void Game::resume_current_level_music() {
    if (m_current_level == nullptr) {
        return;
    }

    std::cout << "[game] resuming music\n";
    SeekMusicStream(m_current_level->music, m_current_level->m_current_music_progress);
    ResumeMusicStream(m_current_level->music);
}

void Game::update_current_level_progress() {
    if (m_paused || m_current_level == nullptr) {
        return;
    }

    UpdateMusicStream(m_current_level->music);

    m_current_level->m_current_music_progress = GetMusicTimePlayed(m_current_level->music);
    if (m_current_level->m_current_progress < 100.0f) m_current_level->update();
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

            const std::string location = file.path().string();
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

    const std::string level_key(location);
    auto level_it = m_levels.find(level_key);

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

    m_paused = false;
    m_was_paused = false;
    m_best_object = nullptr;
    m_ui.mode = mode;
    m_current_level = level;

    std::cout << "loaded " << location << " succefully" << "\n";
}

void Game::unload_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to unload current level (not found)\n";
        return;
    }

    StopMusicStream(m_current_level->music);
    UnloadMusicStream(m_current_level->music);

    m_current_level->unload();

    delete m_player;

    m_ui.mode = UIMode::MENU;
    m_ui.reset_playfield_state();

    m_paused = false;
    m_was_paused = false;
    m_player = nullptr;
    m_best_object = nullptr;
    m_current_level = nullptr;
}

void Game::finish_level() {
    // if we reach the end of the level, make player immortal to prevent bs
    m_player->m_ignore_collision = true;
    m_player->m_finished_level = true;

    m_ui.reset_playfield_state();
    m_ui.m_playfield_container_open = true;
    m_ui.m_show_finished = true;
}

void Game::update_camera_focus(GameObject* obj) {
    if (m_best_object != obj) {
        m_best_object = obj;
    }

    m_focus_y = obj->position.y + obj->dimensions.y / 2.0f;
}

void Game::simulate() {
    if (m_ui.mode != UIMode::PLAYFIELD) return;
    if (m_player == nullptr) return;

    m_player->movement();
    m_player->rb->simulate();

    static GameObject* closest_platform = nullptr;

    if (!m_player->m_finished_level) {
        const float player_center_x = m_player->position.x + m_player->dimensions.x / 2.0f;
        const float player_bottom_y = m_player->position.y + m_player->dimensions.y;

        float closest_platform_distance = CAMERA_PLATFORM_Y_THRESHOLD;

        for (const auto& object : m_objects) {
            if (object->type != ObjectType::PLATFORM) continue;

            const float platform_min_x = object->position.x - CAMERA_PLATFORM_X_THRESHOLD;
            const float platform_max_x = object->position.x + object->dimensions.x + CAMERA_PLATFORM_X_THRESHOLD;

            if (player_center_x < platform_min_x || player_center_x > platform_max_x) {
                continue;
            }

            const float platform_distance = std::fabs(player_bottom_y - object->position.y);

            if (platform_distance >= closest_platform_distance) {
                continue;
            }

            closest_platform = object;
            closest_platform_distance = platform_distance;
        }

        if (closest_platform != nullptr) {
            update_camera_focus(closest_platform);
        } else {
            update_camera_focus(m_player);
        }
    }

    m_camera.target = {m_player->position.x + CAMERA_X_LOOK_AHEAD,
                       d_math::lerp(m_camera.target.y, m_focus_y + CAMERA_Y_LOOK_AHEAD, CAMERA_Y_SMOOTHING)};
    m_camera.offset = {m_window.width / 2.0f, m_window.height / 2.0f};
}

void Game::render() {
    if (m_ui.mode == UIMode::MENU) {
        rlImGuiBegin();
        {
            m_ui.render_main_menu();
        }
        rlImGuiEnd();
    } else if (m_ui.mode == UIMode::PLAYFIELD) {
        BeginMode2D(m_camera);
        {
            // render all visible objects
            for (const auto& object : m_objects) {
                object->render();
            }
        }
        EndMode2D();

        // handle escape
        {
            static bool was_pressed = false;

            if (IsKeyPressed(KEY_ESCAPE) && !was_pressed) {
                m_ui.m_playfield_container_open = !m_ui.m_playfield_container_open;

                if (game.m_can_pause) {
                    game.m_paused = m_ui.m_playfield_container_open;
                    m_ui.m_show_pause = game.m_paused && (!m_ui.m_show_dead || !m_ui.m_show_finished);
                }

                was_pressed = true;
            } else {
                was_pressed = false;
            }
        }

        rlImGuiBegin();
        {
            m_ui.render_debug_ui();
            m_ui.render_playfield_ui();
        }
        rlImGuiEnd();
    }
}
