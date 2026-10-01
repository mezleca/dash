#include "spike.hpp"
#include "../../game/sprite.hpp"

Spike::Spike(World& world) : Entity(world, ObjectType::SPIKE) {
    const b2Vec2 vertices[] = {{0.5f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
    const b2Hull hull = b2ComputeHull(vertices, 3);
    set_shape(b2MakePolygon(&hull, 0.0f));
    add_component<Sprite>(SpriteType::TRIANGLE, Color{255, 0, 0, 255});
}
