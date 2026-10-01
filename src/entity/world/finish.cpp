#include "finish.hpp"
#include "../../game/sprite.hpp"

FinishPulse::FinishPulse(GameObject& object) : Component(object) {}

void FinishPulse::update(float frametime) {
    m_alpha -= frametime;

    if (m_alpha < 0.0f) {
        m_alpha = 1.0f;
    }

    auto* sprite = object().get_component<Sprite>();
    if (sprite != nullptr) {
        sprite->tint.a = static_cast<unsigned char>(255.0f * m_alpha);
    }
}

Finish::Finish(World& world) : Entity(world, ObjectType::END) {
    set_shape(b2Circle{{0.0f, 0.0f}, 256.0f / PHYSICS_PIXELS_PER_METER});

    add_component<Sprite>(SpriteType::CIRCLE, Color{255, 165, 0, 255});
    add_component<FinishPulse>();
}
