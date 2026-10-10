#include "finish.hpp"
#include "core/components/sprite.hpp"

FinishPulse::FinishPulse(GameObject& object, Sprite& sprite) : Component(object, ComponentId::None), m_sprite(sprite) {}

void FinishPulse::update(float frametime) {
    m_alpha -= frametime;

    if (m_alpha < 0.0f) {
        m_alpha = 1.0f;
    }

    m_sprite.tint.a = static_cast<unsigned char>(255.0f * m_alpha);
}

Finish::Finish(World& world) {
    auto& body = add_component<RigidBody>(world, b2_staticBody, true);
    body.set_shape(b2Circle{{0.0f, 0.0f}, 256.0f / PHYSICS_PIXELS_PER_METER});

    auto& sprite = add_component<Sprite>(SpriteType::CIRCLE, Color{255, 165, 0, 255});
    add_component<FinishPulse>(sprite);
}
