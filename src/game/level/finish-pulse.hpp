#pragma once

#include "core/components/component.hpp"

class DashLevel;

class FinishPulse : public Component {
public:
    FinishPulse(GameObject& object, const DashLevel& level);

    void update(float frametime) override;
    void render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const override;

private:
    const DashLevel& m_level;
    float m_phase = 0.0F;
};
