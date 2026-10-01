#include "player.hpp"
#include "../game/sprite.hpp"
#include "../utils/math.hpp"
#include "../game/game.hpp"

#include <algorithm>

constexpr float JUMP_FORCE = 1495.5f;
constexpr float HORIZONTAL_ACCELERATION = 45000.0f / 4.0f;
constexpr float PLAYER_HORIZONTAL_DRAG = 12.0f;

static float get_angle_tilt(float a, float b, float c) {
    float result = (a / b) * c;

    if (result > c) result = c;
    if (result < -c) result = -c;

    return result;
}

Player::Player(World& world, bool god_mode) : Entity(world, ObjectType::BOX, b2_dynamicBody) {
    m_sprite = &add_component<Sprite>();
    update_player_type(PlayerType::BIRD);
    m_in_god_mode = god_mode;
}

void Player::on_contact(Entity& other, Vector2 normal) {
    if (m_in_god_mode || m_dead) {
        return;
    }

    if (other.type() == ObjectType::SPIKE || (other.type() == ObjectType::PLATFORM && normal.y <= 0.5f)) {
        game.kill_player();
        return;
    }

    if (other.type() == ObjectType::PLATFORM) {
        game.update_camera_focus(&other);
    }
}

void Player::reset() {
    set_velocity(0.0f, 0.0f);
    set_collision_enabled(true);

    m_dead = false;
    m_sprite->rotation = 0.0f;
    m_sprite->flip_x = false;
}

void Player::kill() {
    if (!m_in_god_mode) {
        m_dead = true;
    }
}

void Player::update_player_type(PlayerType player_type) {
    switch (player_type) {
        case PlayerType::NONE:
        case PlayerType::BOX: {
            m_sprite->load_texture("resources/sprites/default.png");
            set_gravity(DEFAULT_GRAVITY);
            break;
        }
        case PlayerType::BIRD: {
            m_sprite->load_texture("resources/sprites/bird.png");
            set_gravity(DEFAULT_GRAVITY / 3.0f);
            break;
        }
    }

    m_player_type = player_type;
}

void Player::movement() {
    if (m_dead) return;

    bool is_pressing_jump = IsKeyDown(KEY_SPACE);

    bool is_birb = m_player_type == PlayerType::BIRD;

    float jump_force = JUMP_FORCE;
    int direction = 0;

    Vector2 velocity = get_velocity();

    if (!game.has_finished_level()) {
        // horizontal movement
        if (m_in_free_mode && IsKeyDown(KEY_A)) {
            direction = -1;
        } else if (!m_in_free_mode || IsKeyDown(KEY_D)) {
            direction = 1;
        }
    }

    velocity.x += static_cast<float>(direction) * HORIZONTAL_ACCELERATION * game.fixed_frametime();

    // box2d linear damping also slows jumps, so horizontal drag stays with player input.
    velocity.x *= std::max(0.0f, 1.0f - PLAYER_HORIZONTAL_DRAG * game.fixed_frametime());
    m_sprite->flip_x = direction == -1;

    if (is_birb) {
        jump_force /= 3.0f;
    }

    // vertical movement
    if (!game.has_finished_level() && is_pressing_jump && (is_grounded() || is_birb)) {
        velocity.y = -jump_force;
    }

    set_velocity(velocity.x, velocity.y);

    // update sprite rotation
    if (m_sprite->rotation >= 360.0f) {
        m_sprite->rotation = 0.0f;
    }

    if (!is_grounded() && !game.is_paused()) {
        if (is_birb) {
            m_sprite->rotation = d_math::lerp(m_sprite->rotation, get_angle_tilt(velocity.y, jump_force, 45.0f), 0.25f);
        } else {
            if (velocity.x > 0) {
                m_sprite->rotation += 180.0f * game.fixed_frametime();
            } else {
                m_sprite->rotation -= 180.0f * game.fixed_frametime();
            }
        }
    } else if (!game.is_paused()) {
        float target_angle = is_birb ? 0.0f : std::round(m_sprite->rotation / 90.0f) * 90.0f;
        m_sprite->rotation = d_math::lerp(m_sprite->rotation, target_angle, 0.2f);
    }
}
