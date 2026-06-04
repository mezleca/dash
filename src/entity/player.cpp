#include "player.hpp"
#include "../utils/math.hpp"
#include "../game/game.hpp"
#include "../physics/rigidbody.hpp"

#include <iostream>
#include <raylib.h>

const float JUMP_FORCE = 250.0f;

Player::Player() {
    load_texture("resources/sprites/square.png");

    rb->on_hit = [&](GameObject* obj) {
        if (obj->name == "spike") {
            load_texture("resources/sprites/fireman.png");
            PlaySound(game.explosion);
            game.paused = true;
            shit_rotation = 0.0f;
        }
    };
}

Player::~Player() {
    UnloadTexture(texture);
}

void Player::movement() {
    bool is_pressing_left = IsKeyDown(KEY_A);
    bool is_pressing_right = IsKeyDown(KEY_D) || should_lock_in_horizontally;
    bool is_pressing_jump = IsKeyDown(KEY_SPACE);

    int direction = 0;

    previous_position = position;

    // horizontal movement
    if (is_pressing_left && !should_lock_in_horizontally) {
        direction = -1;
    } else if (is_pressing_right || should_lock_in_horizontally) {
        direction = 1;
    }

    velocity.x += direction * 250.0f;

    // vertical movement
    if (is_pressing_jump && rb->grounded) {
        velocity.y = -JUMP_FORCE;
    }
}

void Player::render() {
    // TODO: actual rotation on jump

    Vector2 interpolated_position = {
        d_math::lerp(previous_position.x, position.x, game.alpha),
        d_math::lerp(previous_position.y, position.y, game.alpha)
    };

    if (game.paused) {
        shit_rotation = d_math::lerp(shit_rotation, 360.0f, 0.05f);

        Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
        Rectangle dest = { interpolated_position.x, interpolated_position.y, (float)texture.width, (float)texture.height };

        Vector2 origin = { (float)texture.width / 2.0f, (float)texture.height / 2.0f };

        DrawTexturePro(
            texture,
            source,
            dest,
            origin,
            shit_rotation,
            WHITE
        );
    } else {
        DrawTextureV(texture, interpolated_position, WHITE);
    }
}
