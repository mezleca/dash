#include "player.hpp"
#include "game/dash.hpp"
#include "core/components/sprite.hpp"
#include "core/components/particles.hpp"
#include "core/math.hpp"
#include "core/camera.hpp"
#include "core/physics/shape.hpp"
#include "game/objects/world/platform.hpp"
#include "game/objects/world/solid.hpp"
#include "game/objects/world/spike.hpp"

#include <algorithm>
#include <cmath>

constexpr float JUMP_FORCE = 1495.5f;
constexpr float HORIZONTAL_ACCELERATION = 45000.0f / 4.0f;
constexpr float PLAYER_HORIZONTAL_DRAG = 12.0f;
constexpr Vector2 CAMERA_LOOK_AHEAD = {128.0f, -128.0f};
constexpr float STEP_DISTANCE = 20.0F;

static float get_angle_tilt(float a, float b, float c) {
    return std::clamp((a / b) * c, -c, c);
}

Player::Player(Dash& game, PlayerType initial_type, bool god_mode)
    : Entity(EntityType::PLAYER), m_game(game), m_body(add_component<RigidBody>(game.world(), b2_dynamicBody)) {
    m_body.set_shape(make_box_shape(SIZE.x, SIZE.y));
    m_particles = &add_component<Particles>(128);
    m_sprite = &add_component<Sprite>();
    update_player_type(initial_type);
    m_in_god_mode = god_mode;
}

void Player::on_contact(GameObject& other, Vector2 normal) {
    if (m_in_god_mode || m_dead || m_frozen) {
        return;
    }

    const bool platform = dynamic_cast<const Platform*>(&other) != nullptr;
    const bool solid = platform || dynamic_cast<const Solid*>(&other) != nullptr;

    if (dynamic_cast<const Spike*>(&other) != nullptr || (solid && normal.y <= 0.5f)) {
        m_death_requested = true;
        return;
    }
}

void Player::reset() {
    m_body.set_velocity(0.0f, 0.0f);
    m_body.set_collision_enabled(true);

    m_dead = false;
    m_frozen = false;
    m_death_requested = false;
    m_sprite->rotation = 0.0f;
    m_sprite->flip_x = false;
    m_camera_focus_y = 0.0f;
    m_step_distance = 0.0F;
    m_particles->clear();
}

// https://raw.githubusercontent.com/enderprism/godot-dash-v2/master/src/PlayerCamera.gd
Vector2 Player::camera_target(const CameraView& camera, const DashLevel& level, bool snap) {
    const Viewport* viewport = m_game.get_viewport(0);
    const Rectangle bounds = m_body.get_bounding_box();
    const float center_y = bounds.y + bounds.height * 0.5F;
    const float height =
        viewport != nullptr && viewport->size().y > 0.0F ? viewport->size().y : static_cast<float>(m_game.window().height);
    const float margin = height * 0.25F / camera.view().zoom;

    if (snap) {
        m_camera_focus_y = center_y + std::clamp(CAMERA_LOOK_AHEAD.y, -margin, margin);
    } else if (!m_game.has_finished_level()) {
        m_camera_focus_y = center_y - std::clamp(center_y - m_camera_focus_y, -margin, margin);
    }

    Vector2 target = {m_body.get_position().x + CAMERA_LOOK_AHEAD.x, m_camera_focus_y};
    if (viewport != nullptr && viewport->size().x > 0.0F) {
        const float half_width = viewport->size().x / (2.0F * camera.view().zoom);
        const float minimum = level.player_start().x + half_width - CAMERA_LOOK_AHEAD.x;
        const float maximum = level.end_position().x - half_width + CAMERA_LOOK_AHEAD.x;

        target.x = minimum <= maximum ? std::clamp(target.x, minimum, maximum) : (minimum + maximum) * 0.5F;
    }

    return target;
}

void Player::kill() {
    m_dead = true;
}

void Player::freeze() {
    m_frozen = true;
    m_body.set_velocity(0.0f, 0.0f);

    const Vector2 position = m_body.get_position();
    m_body.set_position(position.x, position.y);
}

void Player::update_player_type(PlayerType player_type) {
    switch (player_type) {
        case PlayerType::NONE:
        case PlayerType::BOX: {
            m_sprite->load_texture("resources/sprites/default.png");
            m_body.set_gravity(DEFAULT_GRAVITY);
            break;
        }
        case PlayerType::BIRD: {
            m_sprite->load_texture("resources/sprites/bird.png");
            m_body.set_gravity(DEFAULT_GRAVITY / 3.0f);
            break;
        }
    }

    m_player_type = player_type;
}

void Player::movement(float fixed_frametime) {
    if (m_dead || m_frozen) return;

    const bool level_finished = m_game.has_finished_level();

    bool is_pressing_jump = IsKeyDown(KEY_SPACE);
    bool is_birb = m_player_type == PlayerType::BIRD;

    float jump_force = JUMP_FORCE;
    int direction = 0;

    Vector2 velocity = m_body.get_velocity();

    if (!level_finished) {
        // horizontal movement
        if (m_in_free_mode && IsKeyDown(KEY_A)) {
            direction = -1;
        } else if (!m_in_free_mode || IsKeyDown(KEY_D)) {
            direction = 1;
        }
    }

    velocity.x += static_cast<float>(direction) * HORIZONTAL_ACCELERATION * fixed_frametime;

    // damp x without damping jumps.
    velocity.x *= std::max(0.0f, 1.0f - PLAYER_HORIZONTAL_DRAG * fixed_frametime);
    m_sprite->flip_x = direction == -1;

    if (is_birb) {
        jump_force /= 3.0f;
    }

    // vertical movement
    if (!level_finished && is_pressing_jump && (m_body.is_grounded() || is_birb)) {
        velocity.y = -jump_force;
    }

    m_body.set_velocity(velocity.x, velocity.y);
    update_step_particles(fixed_frametime, velocity);

    // update sprite rotation
    if (m_sprite->rotation >= 360.0f) {
        m_sprite->rotation = 0.0f;
    }

    if (!m_body.is_grounded()) {
        if (is_birb) {
            m_sprite->rotation = d_math::lerp(m_sprite->rotation, get_angle_tilt(velocity.y, jump_force, 45.0f), 0.25f);
        } else {
            if (velocity.x > 0) {
                m_sprite->rotation += 180.0f * fixed_frametime;
            } else {
                m_sprite->rotation -= 180.0f * fixed_frametime;
            }
        }
    } else {
        float target_angle = is_birb ? 0.0f : std::round(m_sprite->rotation / 90.0f) * 90.0f;
        m_sprite->rotation = d_math::lerp(m_sprite->rotation, target_angle, 0.2f);
    }
}

void Player::update_step_particles(float timestep, Vector2 velocity) {
    if (m_player_type != PlayerType::BOX || !m_body.is_grounded() || velocity.y < 0.0F || std::abs(velocity.x) < 1.0F) {
        m_step_distance = 0.0F;
        return;
    }

    m_step_distance += std::abs(velocity.x) * timestep;
    if (m_step_distance < STEP_DISTANCE) {
        return;
    }

    m_step_distance = std::fmod(m_step_distance, STEP_DISTANCE);
    const Rectangle bounds = m_body.get_bounding_box();

    Particle particle = {
        .acceleration = {0.0F, 100.0F},
        .color = {200, 200, 200, 160},
        .radius = 4.0F,
        .growth = -6.0F,
        .lifetime = 0.35F,
    };

    particle.radius = static_cast<float>(GetRandomValue(2, 4));
    particle.position = {velocity.x > 0.0F ? bounds.x : bounds.x + bounds.width, bounds.y + bounds.height - particle.radius};
    particle.velocity = {-velocity.x * 0.1F, -static_cast<float>(GetRandomValue(20, 60))};

    m_particles->emit(particle);
}
