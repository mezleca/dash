#pragma once

#include "core/components/rigid-body.hpp"
#include "game/entity/entity.hpp"

class Sprite;

class FinishPulse : public Component {
public:
    FinishPulse(GameObject& object, Sprite& sprite);

    void update(float frametime) override;

private:
    Sprite& m_sprite;
    float m_alpha = 0.0f;
};

class Finish : public Entity {
public:
    explicit Finish(World& world);
};
