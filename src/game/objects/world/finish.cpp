#include "finish.hpp"

#include "core/components/rigid-body.hpp"
#include "core/components/sprite.hpp"
#include "core/physics/shape.hpp"
#include "game/level/level.hpp"

Finish::Finish(World& world, DashLevel& level) : Entity(EntityType::FINISH), m_level(level) {
    auto& body = add_component<RigidBody>(world, b2_staticBody, true);
    body.set_shape(make_box_shape(128.0F, 1024.0F));
    add_component<Sprite>(SpriteType::SQUARE, ORANGE);
    add_behaviour(BehaviourType::Pulse);
}

void Finish::on_sensor(GameObject& visitor) {
    m_level.request_finish(visitor);
}
