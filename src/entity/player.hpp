#pragma once

#include "entity.hpp"

enum class PlayerType : int {
    NONE = -1,
    BOX,
    BIRD
};

class Player : public Entity {
public:
    explicit Player(World& world);

    float m_rotation = 0.0f;
    PlayerType m_player_type;

    bool m_ignore_collision = false;
    bool m_dead = false;
    bool m_should_lock_in_horizontally = false;

    void movement();
    void update_player_type(PlayerType player_type);
    void reset();

    void on_contact(Entity& other, Vector2 normal) override;

    void render() override;

private:
    bool m_should_flip_player = false;
};
