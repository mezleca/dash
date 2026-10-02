#pragma once

#include "entity.hpp"

class Sprite;
class GameCamera;

enum class PlayerType : int {
    NONE = -1,
    BOX,
    BIRD
};

class Player : public Entity {
public:
    explicit Player(World& world, bool god_mode = false);

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

    void movement();
    void update_camera(GameCamera& camera, const World& world, bool level_finished);

    void update_player_type(PlayerType player_type);

    void reset();
    void freeze();
    void kill();

    void on_contact(Entity& other, Vector2 normal) override;

private:
    void update_camera_focus(const Entity& entity);

    Sprite* m_sprite = nullptr;
    PlayerType m_player_type = PlayerType::BIRD;

    bool m_in_god_mode = false;
    bool m_in_free_mode = false;
    bool m_dead = false;
    bool m_frozen = false;
    float m_camera_focus_y = 0.0f;
};
