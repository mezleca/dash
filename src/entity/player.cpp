#include "player.hpp"
#include "../utils/math.hpp"
#include "../game/game.hpp"
#include "../physics/rigidbody.hpp"

constexpr float JUMP_FORCE = 1495.5f;

Player::Player() : GameObject(ObjectType::BOX) {
    load_texture("resources/sprites/default.png");

    rb->on_hit = [&](GameObject* obj) {
        if (obj->type == ObjectType::PLAFORM) {
            last_platform = obj;
        }
    };
}

Player::~Player() {
    UnloadTexture(texture);
}

void Player::movement() {
    if (!visible) return;

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

    velocity.x += direction * 920.0f;

    // vertical movement
    if (is_pressing_jump && rb->grounded) {
        velocity.y = -JUMP_FORCE;
        last_platform = nullptr;
    }
}

void Player::render() {
    if (!visible) return;

    Vector2 interpolated_position = {d_math::lerp(previous_position.x, position.x, game.alpha),
                                     d_math::lerp(previous_position.y, position.y, game.alpha)};

    float texture_width = static_cast<float>(texture.width);
    float texture_height = static_cast<float>(texture.height);

    Rectangle source = {0.0f, 0.0f, texture_width, texture_height};

    Rectangle dest = {interpolated_position.x + texture_width / 2.0f, interpolated_position.y + texture_height / 2.0f,
                      texture_width, texture_height};

    Vector2 origin = {texture_width / 2.0f, texture_height / 2.0f};

    if (m_rotation >= 360.0f) {
        m_rotation = 0.0f;
    }

    if (!rb->grounded) {
        m_rotation += 180.0f * game.fixed_timestep;
    } else {
        float target_angle = std::round(m_rotation / 90.0f) * 90.0f;
        m_rotation = d_math::lerp(m_rotation, target_angle, 0.2f);
    }

    DrawTexturePro(texture, source, dest, origin, m_rotation, WHITE);
}
