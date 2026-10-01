#include "finish.hpp"

#include <raylib.h>

Finish::Finish(World& world) : Entity(world, ObjectType::END) {
    m_radius = 256.0f;
    set_shape(make_box_shape(256.0f, 256.0f));
}

void Finish::render() {
    if (!visible) return;

    m_lum -= 1.0f * GetFrameTime();

    if (m_lum < 0.0f) {
        m_lum = 1.0f;
    }

    DrawCircleV(get_position(), m_radius, {255, 165, 0, static_cast<unsigned char>(m_lum * 255.0f)});
}
