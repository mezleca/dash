#include "platform.hpp"
#include "core/components/sprite.hpp"
#include "core/physics/shape.hpp"

Platform::Platform(World& world) : Entity(EntityType::PLATFORM) {
    add_component<RigidBody>(world).set_shape(make_box_shape(256.0F, 32.0F));
    add_component<Sprite>(SpriteType::SQUARE, Color{0, 120, 255, 255});
}
