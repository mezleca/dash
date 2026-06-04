#include "player.hpp"
#include "../game/game.hpp"
#include "../physics/rigidbody.hpp"

#include <raylib.h>

const float JUMP_FORCE = 175.0f;

Player::Player() {
    load_texture("resources/sprites/square.png");
}

void Player::jump() {
}

void Player::movement() {
    bool is_pressing_left = IsKeyDown(KEY_A);
    bool is_pressing_right = IsKeyDown(KEY_D);
    bool is_pressing_jump = IsKeyDown(KEY_SPACE);

    int direction = 0;
    float speed = 0.0f;

    previous_position = position;

    // horizontal movement
    if (is_pressing_left && is_pressing_right) {
        // do nothing
    } else if (is_pressing_left) {
        speed = 100.0f;
        direction = -1;
    } else if (is_pressing_right) {
        speed = 100.0f;
        direction = 1;
    }

    // vertical movement
    if (is_pressing_jump && rb->grounded) {
        velocity.y = -JUMP_FORCE;
    }

    position.x += direction * speed * game.fixed_timestep;
    position.y += velocity.y * game.fixed_timestep;
}

void Player::render() {
    DrawRectangleV(position, dimensions, {255, 255, 0, 255});
    DrawTextureV(texture, position, {255, 255, 255, 255});
}
