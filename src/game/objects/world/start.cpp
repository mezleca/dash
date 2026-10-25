#include "start.hpp"

#include "core/components/rigid-body.hpp"
#include "core/physics/shape.hpp"
#include "game/objects/entities/player.hpp"
#include "invisible-sprite.hpp"

Start::Start(World& world, DashLevel& level) : Entity(EntityType::START) {
    auto& body = add_component<RigidBody>(world, b2_staticBody, true, false);
    body.set_shape(make_box_shape(Player::SIZE.x, Player::SIZE.y));
    add_component<InvisibleSprite>(level, "start", GREEN);
}
