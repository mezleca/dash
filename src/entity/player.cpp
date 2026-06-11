#include "player.hpp"
#include "../utils/math.hpp"
#include "../game/game.hpp"
#include "../physics/rigidbody.hpp"
#include <iostream>

constexpr float JUMP_FORCE = 1495.5f;
constexpr float HORIZONTAL_ACCELERATION = 45000.0f / 4.0f;

Player::Player() : GameObject(ObjectType::BOX) {
    game.add_game_object(this);
    load_texture("resources/sprites/default.png");

    rb->on_hit = [&](GameObject* obj) {
        if (m_finished_level) return;

        if (obj->type == ObjectType::PLATFORM) {
            if (!rb->grounded) {
                std::cout << "[player] should be dead\n";
            } else {
                game.update_camera_focus(obj);
            }
        }

        if (obj->type == ObjectType::SPIKE) {
            std::cout << "[player] should be dead\n";
        }
    };
}

Player::~Player() {
    UnloadTexture(texture);
}

void Player::movement() {
    if (!visible) return;

    bool is_pressing_left = IsKeyDown(KEY_A);
    bool is_pressing_right = IsKeyDown(KEY_D) || m_should_lock_in_horizontally;
    bool is_pressing_jump = IsKeyDown(KEY_SPACE);

    int direction = 0;

    previous_position = position;

    if (!m_finished_level) {
        // horizontal movement
        if (is_pressing_left && !m_should_lock_in_horizontally) {
            direction = -1;
        } else if (is_pressing_right || m_should_lock_in_horizontally) {
            direction = 1;
        }
    }

    velocity.x += static_cast<float>(direction) * HORIZONTAL_ACCELERATION * game.m_fixed_frametime;

    // vertical movement
    if (!m_finished_level && is_pressing_jump && rb->grounded) {
        velocity.y = -JUMP_FORCE;
    }

    // update sprite rotation
    if (m_rotation >= 360.0f) {
        m_rotation = 0.0f;
    }

    if (!rb->grounded && !game.m_paused) {
        if (velocity.x > 0) {
            m_rotation += 180.0f * game.m_fixed_frametime;
        } else {
            m_rotation -= 180.0f * game.m_fixed_frametime;
        }
    } else {
        if (!game.m_paused) {
            float target_angle = std::round(m_rotation / 90.0f) * 90.0f;
            m_rotation = d_math::lerp(m_rotation, target_angle, 0.2f);
        }
    }
}

void Player::render() {
    if (!visible) return;

    Vector2 interpolated_position = {d_math::lerp(previous_position.x, position.x, game.m_alpha),
                                     d_math::lerp(previous_position.y, position.y, game.m_alpha)};

    float texture_width = static_cast<float>(texture.width);
    float texture_height = static_cast<float>(texture.height);

    Rectangle source = {0.0f, 0.0f, texture_width, texture_height};

    Rectangle dest = {interpolated_position.x + texture_width / 2.0f, interpolated_position.y + texture_height / 2.0f,
                      texture_width, texture_height};

    Vector2 origin = {texture_width / 2.0f, texture_height / 2.0f};

    DrawTexturePro(texture, source, dest, origin, m_rotation, WHITE);
}
