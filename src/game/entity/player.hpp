#pragma once

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"

class Sprite;
class CameraView;
class Dash;

enum class PlayerType : int {
    NONE = -1,
    BOX,
    BIRD
};

class Player : public GameObject {
public:
    explicit Player(Dash& game, bool god_mode = false);

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
    void update_camera(CameraView& camera, bool snap = false);

    void update_player_type(PlayerType player_type);

    void reset();
    void freeze();
    void kill();

    void on_contact(GameObject& other, Vector2 normal) override;

    bool death_requested() const {
        return m_death_requested;
    }

private:
    void update_camera_focus(const RigidBody& body);

    Dash& m_game;
    RigidBody& m_body;
    Sprite* m_sprite = nullptr;
    PlayerType m_player_type = PlayerType::BIRD;

    bool m_in_god_mode = false;
    bool m_in_free_mode = false;
    bool m_dead = false;
    bool m_frozen = false;
    bool m_death_requested = false;
    float m_camera_focus_y = 0.0f;
};
