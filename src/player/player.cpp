#include "player.hpp"
#include "../game/game.hpp"
#include "../utils/math.hpp"

#include <raylib.h>

const float GRAVITY = 400.0f;
const float FALL_GRAVITY_MULT = 1.35f;

Entity::Entity() {
    rb = new RigidBody(this);
}

GameObject::GameObject() {
    g_game.add_game_object(this);
}

GameObject::~GameObject() {
    g_game.remove_game_object(this);
}

void RigidBody::simulate() {
    float gravity = GRAVITY;

    if (entity->velocity.y > 0.0f) {
        gravity *= FALL_GRAVITY_MULT;
    }

    entity->velocity.y += gravity * g_game.fixed_timestep;

    // placeholder
    if (entity->position.y >= 20) {
        entity->position.y = 20;
        entity->velocity.y = 0.0f;
        grounded = true;
    } else {
        grounded = false;
    }
}

void Player::jump() {
}

void Player::movement() {
    bool is_pressing_left = IsKeyDown(KEY_A);
    bool is_pressing_right = IsKeyDown(KEY_D);
    bool is_pressing_jump = IsKeyDown(KEY_SPACE);

    int direction = 0;
    float speed = 0.0f;

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
        velocity.y = -150.0f;
    }

    position.x += direction * speed * g_game.fixed_timestep;
    position.y += velocity.y * g_game.fixed_timestep;
}

void Player::render() {
    DrawRectangleV(position, {static_cast<float>(width), static_cast<float>(height)}, Color{255, 255, 255, 255});
}
