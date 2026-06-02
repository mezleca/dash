#include "game.hpp"

#include <raylib.h>
#include <vector>

Game::Game() {
    window.title = "dash";
    window.width = 1280;
    window.height = 720;

    camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 5.0f;

    // TEMP:
    ui.state |= UIState::PLAYFIELD;
    player.width = 32;
    player.height = 32;

    for (int i = 0; i < 256; i++) {
        float width = 60;
        float height = 140;
        unsigned char r = std::rand() % 256;
        unsigned char g = std::rand() % 256;
        unsigned char b = std::rand() % 256;

        Rectangle rect{i * width + 30.0f, 10.0f, width, height};
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
        accumulator += GetFrameTime();

        while (accumulator >= fixed_timestep) {
            accumulator -= fixed_timestep;
            simulate();
        }

        alpha = accumulator / fixed_timestep;

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

    player.last_x = player.position.x;
    player.last_y = player.position.y;

    player.rb->simulate();
    player.movement();

    camera.target = player.position;
    camera.offset = (Vector2){static_cast<float>(player.width) + 20.0f, static_cast<float>(player.height) + 40.0f};
}

void Game::render() {
    // TEMP:
    for (const auto& rect : rects) {
        DrawRectangleRec(rect.rect, rect.color);
    }

    player.render();
}
