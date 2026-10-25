#include "solid.hpp"
#include "core/components/rigid-body.hpp"
#include "core/components/sprite.hpp"

#include <algorithm>

Solid::Solid(World& world) : Entity(EntityType::SOLID) {
    add_component<RigidBody>(world);
    add_component<Sprite>(SpriteType::SQUARE, Color{120, 130, 150, 255});
}

bool Solid::set_shape(SpriteType type) {
    auto& body = *get_component<RigidBody>();

    const Rectangle bounds = body.get_bounding_box();
    const Vector2 position = body.get_position();

    // world pixels to local meters
    const float x = (bounds.x - position.x) / PHYSICS_PIXELS_PER_METER;
    const float y = (bounds.y - position.y) / PHYSICS_PIXELS_PER_METER;
    const float width = bounds.width / PHYSICS_PIXELS_PER_METER;
    const float height = bounds.height / PHYSICS_PIXELS_PER_METER;
    const b2Vec2 center = {x + width * 0.5F, y + height * 0.5F};

    switch (type) {
        case SpriteType::SQUARE:
            body.set_shape(b2MakeOffsetBox(width * 0.5F, height * 0.5F, center, b2Rot_identity));
            break;

        case SpriteType::TRIANGLE: {
            const b2Vec2 vertices[] = {{center.x, y}, {x, y + height}, {x + width, y + height}};
            const b2Hull hull = b2ComputeHull(vertices, 3);

            if (hull.count != 3) {
                TraceLog(LOG_WARNING, "[solid] bounds are too small for a triangle collider");
                return false;
            }

            body.set_shape(b2MakePolygon(&hull, 0.0F));
            break;
        }

        case SpriteType::CIRCLE:
            body.set_shape(b2Circle{center, std::min(width, height) * 0.5F});
            break;

        default:
            return false;
    }

    get_component<Sprite>()->type = type;
    return true;
}
