#pragma once

#include "../entity.hpp"

class FinishPulse : public Component {
public:
    explicit FinishPulse(GameObject& object);

    void update(float frametime) override;

private:
    float m_alpha = 0.0f;
};

class Finish : public Entity {
public:
    explicit Finish(World& world);
};
