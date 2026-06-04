#pragma once

#include "entity.hpp"

struct Player : public Entity {
    explicit Player();
    ~Player();

    float shit_rotation = 360.0f;
    bool should_lock_in_horizontally = false;

    void movement();
    void render() override;
};
