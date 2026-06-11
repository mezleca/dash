#include "../../game/game.hpp"
#include "finish.hpp"
#include "../../physics/rigidbody.hpp"

#include <raylib.h>

Finish::Finish() : GameObject(ObjectType::END) {
    game.add_game_object(this);

    rb->is_static = true;
    rb->is_trigger = true;

    m_radius = 256.0f;

    dimensions.x = 256.0f;
    dimensions.y = 256.0f;
}

void Finish::render() {
    if (!visible) return;

    m_lum -= 1.0f * GetFrameTime();

    if (m_lum < 0.0f) {
        m_lum = 1.0f;
    }

    DrawCircleV(position, m_radius, {255, 165, 0, static_cast<unsigned char>(m_lum * 255.0f)});
}
