#include "trigger.hpp"
#include "invisible-sprite.hpp"
#include "core/components/rigid-body.hpp"

Trigger::Trigger(World& world, DashLevel& level) : Entity(EntityType::TRIGGER) {
    add_component<RigidBody>(world, b2_staticBody, true);
    add_component<InvisibleSprite>(level, "trigger");
    set_behaviours_active(false);
}

void Trigger::reset_behaviours() {
    Entity::reset_behaviours();
    set_behaviours_active(false);
}

void Trigger::on_sensor(GameObject&) {
    Entity::reset_behaviours();
}
