#include "pulse.hpp"
#include "core/components/sprite.hpp"

#include <cmath>

Pulse::Pulse(Sprite& sprite, float period, float minimum_opacity)
    : m_sprite(sprite), m_period(period), m_minimum_opacity(minimum_opacity) {}

void Pulse::reset() {
    Behaviour::reset();

    m_phase = 0.0F;
    m_sprite.opacity = 1.0F;
}

void Pulse::serialize(Serializer& serializer) const {
    serializer.field("period", m_period);
    serializer.field("minimum_opacity", m_minimum_opacity);
}

void Pulse::deserialize(Deserializer& deserializer) {
    float period = 1.2F;
    float opacity = 0.35F;
    deserializer.field("period", period);
    deserializer.field("minimum_opacity", opacity);
    set_period(period);
    set_minimum_opacity(opacity);
}

void Pulse::update(float frametime) {
    m_phase = std::fmod(m_phase + frametime / m_period, 1.0F);
    const float intensity = (std::sin(m_phase * 2.0F * PI) + 1.0F) * 0.5F;
    m_sprite.opacity = m_minimum_opacity + intensity * (1.0F - m_minimum_opacity);
}
