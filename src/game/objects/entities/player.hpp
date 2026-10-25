#pragma once

#include "core/components/rigid-body.hpp"
#include "game/objects/entities/entity.hpp"
#include "player-type.hpp"

class Sprite;
class Particles;
class CameraView;
class Dash;
class DashLevel;

class Player : public Entity {
public:
    static constexpr Vector2 SIZE = {64.0F, 64.0F};

    Player(Dash& game, PlayerType initial_type, bool god_mode = false);

    RigidBody& body() const {
        return m_body;
    }

    bool alive() const {
        return !m_dead;
    }

    bool in_god_mode() const {
        return m_in_god_mode;
    }

    void set_god_mode(bool value) {
        m_in_god_mode = value;
    }

    void set_free_mode(bool value) {
        m_in_free_mode = value;
    }

    bool in_free_mode() const {
        return m_in_free_mode;
    }

    void movement(float fixed_frametime);
    Vector2 camera_target(const CameraView& camera, const DashLevel& level, bool snap = false);

    void update_player_type(PlayerType player_type);

    void reset();
    void freeze();
    void kill();

    void on_contact(GameObject& other, Vector2 normal) override;

    bool death_requested() const {
        return m_death_requested;
    }

private:
    void update_step_particles(float timestep, Vector2 velocity);

    Dash& m_game;
    RigidBody& m_body;
    Sprite* m_sprite = nullptr;
    Particles* m_particles = nullptr;
    PlayerType m_player_type = PlayerType::NONE;

    bool m_in_god_mode = false;
    bool m_in_free_mode = false;
    bool m_dead = false;
    bool m_frozen = false;
    bool m_death_requested = false;
    float m_camera_focus_y = 0.0f;
    float m_step_distance = 0.0F;
};
