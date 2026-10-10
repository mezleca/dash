#include "platform.hpp"
#include "core/components/sprite.hpp"

Platform::Platform(World& world) {
    add_component<RigidBody>(world);
    add_component<Sprite>(SpriteType::SQUARE, Color{0, 120, 255, 255});
}
