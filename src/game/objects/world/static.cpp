#include "static.hpp"
#include "image-sprite.hpp"

StaticTexture::StaticTexture(World& world) : Entity(EntityType::STATIC_TEXTURE) {
    add_component<RigidBody>(world, b2_staticBody, false, false);
    add_component<ImageSprite>();
}
