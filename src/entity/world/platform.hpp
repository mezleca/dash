#pragma once

#include "../entity.hpp"

struct Platform : public Entity {
    explicit Platform(float width, float height);

    void render() override;
};
