#include "platform.hpp"
#include "../../game/sprite.hpp"

Platform::Platform(World& world) : Entity(world, ObjectType::PLATFORM) {
    add_component<Sprite>(SpriteType::SQUARE, Color{0, 120, 255, 255});
}
