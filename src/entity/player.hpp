#pragma once

#include "entity.hpp"

struct Player : public Entity {
    explicit Player();

    void movement();
    void jump();
    void render() override;
};
