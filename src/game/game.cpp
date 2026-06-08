#include "game.hpp"
#include "../entity/player.hpp"
#include "../physics/rigidbody.hpp"
#include "../entity/world/platform.hpp"
#include "../entity/world/spike.hpp"
#include "../utils/math.hpp"

#include <iostream>
#include <rlImGui.h>
#include <raylib.h>

GameObject::GameObject() {
    game.add_game_object(this);
}

GameObject::~GameObject() {
    game.remove_game_object(this);
}

void GameObject::load_texture(const char* location) {
    if (texture.id) UnloadTexture(texture);

    texture = LoadTexture(location);

    dimensions.x = static_cast<float>(texture.width);
    dimensions.y = static_cast<float>(texture.height);
}

Game::Game() {
    ui.mode = UIMode::MENU;

    window.title = "dash";
    window.width = 1280;
    window.height = 720;

    camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 1.2f;

    // TEMP:
    auto floor = new Platform(2056 * 2, 30);
    floor->position.x = 0.0f;
    floor->position.y = 100.0f;

    auto spike1 = new Spike(3);
    spike1->position.y = 100.0f - spike1->dimensions.y;
    spike1->position.x = spike1->dimensions.x + 350.0f;

    auto spike2 = new Spike(2);
    spike2->position.y = 100.0f - spike1->dimensions.y;
    spike2->position.x = spike1->dimensions.x + 700.0f;

    auto spike3 = new Spike(3);
    spike3->position.y = 100.0f - spike1->dimensions.y;
    spike3->position.x = spike1->dimensions.x + 900.0f;

    auto spike4 = new Spike(2);
    spike4->position.y = 100.0f - spike1->dimensions.y;
    spike4->position.x = spike1->dimensions.x + 1100.0f;

    auto spike5 = new Spike(2);
    spike5->position.y = 100.0f - spike1->dimensions.y;
    spike5->position.x = spike1->dimensions.x + 1250.0f;

    auto spike6 = new Spike(2);
    spike6->position.y = 100.0f - spike1->dimensions.y;
    spike6->position.x = spike1->dimensions.x + 1550.0f;

    for (int i = 0; i < 256; i++) {
        float width = 32;
        float height = width * 3;

        Rectangle rect{i * width + (i * 10.0f), 10.0f, width, height};
        Color color{0, 255, 0, 255};

        rects.push_back({rect, color});
    }
}

Game::~Game() {
}

void Game::initialize() {
    InitWindow(window.width, window.height, window.title.c_str());
    InitAudioDevice();

    SetTargetFPS(60);

    rlImGuiSetup(true);

    // TEMP:
    explosion = LoadSound("resources/songs/creeper.mp3");

    test_level_music = LoadMusicStream("resources/songs/fireman.mp3");
    SetMusicPan(test_level_music, 0.0f);
    SetMusicVolume(test_level_music, 0.5f);

    player = new Player();
    player->should_lock_in_horizontally = false;
    player->position.y = 100.0f - player->dimensions.y;
    // ----

    while (!WindowShouldClose()) {
        UpdateMusicStream(test_level_music);

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

void Game::simulate() {
    if (IsKeyDown(KEY_R)) {
        player->load_texture("resources/sprites/default.png");
        SeekMusicStream(test_level_music, 0.0f);
        paused = false;
        player->position = { 0, 0 };
        player->velocity = { 0, 0 };
    }

    if (paused) return;
    if (ui.mode != UIMode::PLAYFIELD) return;

    player->rb->simulate();
    player->movement();

    camera.target = {
        d_math::lerp(camera.target.x, player->position.x + 20.0f, 0.2f),
        player->position.y
    };

    camera.offset = {player->dimensions.x + 20.0f, player->dimensions.y + 40.0f};
}

void Game::render() {
    if (ui.mode == UIMode::MENU) {
        ui.render_main_menu();
    } else if (ui.mode == UIMode::PLAYFIELD) {
        BeginMode2D(camera);
        {
            // TEMP:
            for (const auto& rect : rects) {
                DrawRectangleRec(rect.rect, rect.color);
            }
            // -----

            for (const auto& object : m_objects) {
                object->render();
            }

            player->render();
        }
        EndMode2D();
    }
}
