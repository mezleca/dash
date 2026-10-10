#pragma once

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"

class Sprite;

class FinishPulse : public Component {
public:
    FinishPulse(GameObject& object, Sprite& sprite);

    void update(float frametime) override;

private:
    Sprite& m_sprite;
    float m_alpha = 0.0f;
};

class Finish : public GameObject {
public:
    explicit Finish(World& world);
};
