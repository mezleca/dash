#pragma once

#include "entity.hpp"

struct Player : public Entity {
    explicit Player();
    ~Player();

    float m_rotation = 0.0f;
    GameObject* last_floor;

    bool should_lock_in_horizontally = false;

    void movement();
    void render() override;
};
