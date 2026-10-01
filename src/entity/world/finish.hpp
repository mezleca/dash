#pragma once

#include "../entity.hpp"

class Finish : public Entity {
public:
    explicit Finish(World& world);

    float m_radius = 0.0f;
    float m_lum = 0.0f;

    void render() override;
};
