#include "game.hpp"
#include "../entity/player.hpp"
#include "../physics/rigidbody.hpp"
#include "../entity/platform.hpp"

#include <iostream>
#include <raylib.h>

GameObject::GameObject() {
    game.add_game_object(this);
}

GameObject::~GameObject() {
    game.remove_game_object(this);
}

void GameObject::load_texture(const char* location) {
    texture = LoadTexture(location);

    dimensions.x = static_cast<float>(texture.width);
    dimensions.y = static_cast<float>(texture.height);
}

Game::Game() {
    // setup player object
    player = new Player();

    window.title = "dash";
    window.width = 1280;
    window.height = 720;

    camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;

    // TEMP:
    floor = new Platform(128, 10);
    floor->position.x = 0.0f;
    floor->position.y = 100.0f;

    ui.state |= UIState::PLAYFIELD;

    for (int i = 0; i < 256; i++) {
        float width = 60;
        float height = 140;
        unsigned char r = std::rand() % 256;
        unsigned char g = std::rand() % 256;
        unsigned char b = std::rand() % 256;

        Rectangle rect{i * width + (i * 15.0f), 10.0f, width, height};
        Color color{r, g, b, 255};

        rects.push_back({rect, color});
    }
}

Game::~Game() {
}

void Game::initialize() {
    InitWindow(window.width, window.height, window.title.c_str());
    SetTargetFPS(60);

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

            BeginMode2D(camera);
            {
                render();
            }
            EndMode2D();
        }
        EndDrawing();
    }

    CloseWindow();
}

void Game::simulate() {
    if (!ui.is_on_playfield()) return;

    player->rb->simulate();
    player->movement();

    camera.target = {0, 0};
    camera.offset = {player->dimensions.x + 20.0f, player->dimensions.y + 40.0f};
}

void Game::render() {
    // TEMP:
    for (const auto& rect : rects) {
        DrawRectangleRec(rect.rect, rect.color);
    }

    floor->render();
    // -----

    player->render();
}
