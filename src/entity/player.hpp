#pragma once

#include "entity.hpp"

class Sprite;

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

    void update_player_type(PlayerType player_type);

    void reset();
    void kill();

    void on_contact(Entity& other, Vector2 normal) override;

private:
    Sprite* m_sprite = nullptr;
    PlayerType m_player_type = PlayerType::BIRD;

    bool m_in_god_mode = false;
    bool m_in_free_mode = false;
    bool m_dead = false;
};
