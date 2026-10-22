#include "finish-pulse.hpp"
#include "level.hpp"

#include <cmath>
#include <algorithm>

FinishPulse::FinishPulse(GameObject& object, const DashLevel& level) : Component(object, ComponentId::None), m_level(level) {}

void FinishPulse::update(float frametime) {
    if (m_level.state() == LevelState::PLAYING) {
        m_phase = std::fmod(m_phase + frametime / 1.2F, 1.0F);
    }
}

void FinishPulse::render(Rectangle, const Camera2D& camera, Vector2 viewport_size) const {
    if (!m_level.loaded() || m_level.state() == LevelState::EDITING) {
        return;
    }

    const Vector2 corners[] = {
        GetScreenToWorld2D({0, 0}, camera), GetScreenToWorld2D({viewport_size.x, 0}, camera),
        GetScreenToWorld2D(viewport_size, camera), GetScreenToWorld2D({0, viewport_size.y}, camera)
    };

    Vector2 minimum = corners[0];
    Vector2 maximum = corners[0];

    for (const Vector2 corner : corners) {
        minimum = {std::min(minimum.x, corner.x), std::min(minimum.y, corner.y)};
        maximum = {std::max(maximum.x, corner.x), std::max(maximum.y, corner.y)};
    }

    const float left = std::max(m_level.end_position().x, minimum.x);
    if (left >= maximum.x) {
        return;
    }

    const Rectangle bounds = {left, minimum.y, maximum.x - left, maximum.y - minimum.y};
    const float intensity = (std::sin(m_phase * 2.0F * PI) + 1.0F) * 0.5F;
    DrawRectangleRec(bounds, Fade(ORANGE, 0.35F + intensity * 0.65F));
}
