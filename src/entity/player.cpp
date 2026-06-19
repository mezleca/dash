#include "player.hpp"
#include "../utils/math.hpp"
#include "../game/game.hpp"
#include "../physics/rigidbody.hpp"

constexpr float JUMP_FORCE = 1495.5f;
constexpr float HORIZONTAL_ACCELERATION = 45000.0f / 4.0f;

static float get_angle_tilt(float a, float b, float c) {
    float result = (a / b) * c;

    if (result > c) result = c;
    if (result < -c) result = -c;

    return result;
}

Player::Player() : GameObject(ObjectType::BOX) {
    update_player_type(PlayerType::BIRD);

    rb->on_hit = [&](GameObject* obj) {
        if (m_finished_level || m_dead) return;

        if (obj->type == ObjectType::PLATFORM) {
            if (!rb->grounded) {
                game.kill_player();
            } else {
                game.update_camera_focus(obj);
            }
        }

        if (obj->type == ObjectType::SPIKE) {
            game.kill_player();
        }
    };

    game.add_game_object(this);
}

Player::~Player() {
}

void Player::reset() {
    velocity = {0, 0};
    m_rotation = 0.0f;
    m_dead = false;
    m_finished_level = false;
    m_should_flip_player = false;
}

void Player::update_player_type(PlayerType player_type) {
    switch (player_type) {
        case PlayerType::NONE:
        case PlayerType::BOX: {
            load_texture("resources/sprites/default.png");
            rb->m_gravity = DEFAULT_GRAVITY;
            break;
        }
        case PlayerType::BIRD: {
            load_texture("resources/sprites/bird.png");
            rb->m_gravity = DEFAULT_GRAVITY / 3.0f;
            break;
        }
    }

    m_player_type = player_type;
}

void Player::movement() {
    if (!visible) return;
    if (m_dead) return;

    bool is_pressing_left = IsKeyDown(KEY_A);
    bool is_pressing_right = IsKeyDown(KEY_D) || m_should_lock_in_horizontally;
    bool is_pressing_jump = IsKeyDown(KEY_SPACE);

    bool is_birb = m_player_type == PlayerType::BIRD;

    float jump_force = JUMP_FORCE;
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
    m_should_flip_player = direction == -1;

    if (is_birb) {
        jump_force /= 3.0f;
    }

    // vertical movement
    if (!m_finished_level && is_pressing_jump && (rb->grounded || is_birb)) {
        velocity.y = -jump_force;
    }

    // update sprite rotation
    if (m_rotation >= 360.0f) {
        m_rotation = 0.0f;
    }

    if (!rb->grounded && !game.m_paused) {
        if (is_birb) {
            m_rotation = d_math::lerp(m_rotation, get_angle_tilt(velocity.y, jump_force, 45.0f), 0.25f);
        } else {
            if (velocity.x > 0) {
                m_rotation += 180.0f * game.m_fixed_frametime;
            } else {
                m_rotation -= 180.0f * game.m_fixed_frametime;
            }
        }
    } else if (!game.m_paused) {
        float target_angle = is_birb ? 0.0f : std::round(m_rotation / 90.0f) * 90.0f;
        m_rotation = d_math::lerp(m_rotation, target_angle, 0.2f);
    }
}

void Player::render() {
    if (!visible) return;

    Vector2 interpolated_position = {d_math::lerp(previous_position.x, position.x, game.m_alpha),
                                     d_math::lerp(previous_position.y, position.y, game.m_alpha)};

    float texture_width = static_cast<float>(texture.width);
    float texture_height = static_cast<float>(texture.height);

    Rectangle source = {0.0f, 0.0f, m_should_flip_player ? -texture_width : texture_width, texture_height};

    Rectangle dest = {interpolated_position.x + texture_width / 2.0f, interpolated_position.y + texture_height / 2.0f,
                      texture_width, texture_height};

    Vector2 origin = {texture_width / 2.0f, texture_height / 2.0f};

    DrawTexturePro(texture, source, dest, origin, m_should_flip_player ? -m_rotation : m_rotation, WHITE);
}
