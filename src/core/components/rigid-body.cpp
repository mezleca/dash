#include "rigid-body.hpp"
#include "core/object.hpp"
#include "core/physics/shape.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <variant>

template <typename Transform>
static b2AABB compute_aabb(const PhysicsShape& base_shape, const Transform& transform) {
    return std::visit(
        [&transform](const auto& shape) {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, b2Polygon>) {
                return b2ComputePolygonAABB(&shape, transform);
            } else if constexpr (std::is_same_v<T, b2Circle>) {
                return b2ComputeCircleAABB(&shape, transform);
            } else {
                return b2ComputeCapsuleAABB(&shape, transform);
            }
        },
        base_shape
    );
}

static Rectangle to_rectangle(const b2AABB& bounds) {
    // convert the tight box2d bounds from meters to raylib pixels.
    return {
        bounds.lowerBound.x * PHYSICS_PIXELS_PER_METER, bounds.lowerBound.y * PHYSICS_PIXELS_PER_METER,
        (bounds.upperBound.x - bounds.lowerBound.x) * PHYSICS_PIXELS_PER_METER,
        (bounds.upperBound.y - bounds.lowerBound.y) * PHYSICS_PIXELS_PER_METER
    };
}

RigidBody::RigidBody(GameObject& object, World& world, b2BodyType physics_type, bool sensor, bool collidable)
    : Component(object, ComponentId::RigidBody), m_world(world), m_collidable(collidable) {
    b2BodyDef definition = b2DefaultBodyDef();
    definition.type = physics_type;
    definition.motionLocks.angularZ = true;
    definition.enableSleep = false;
    definition.enableContactRecycling = false;
    m_body = b2CreateBody(m_world.id(), &definition);

    m_base_shape = make_box_shape(64.0f, 64.0f);
    m_shape = create_shape(shape_definition(sensor));

    m_world.add(*this);
}

RigidBody::~RigidBody() {
    m_world.remove(*this);

    // the world may already have destroyed the body
    if (b2Body_IsValid(m_body)) {
        b2DestroyBody(m_body);
    }
}

Vector2 RigidBody::get_position() const {
    const b2Pos position = b2Body_GetPosition(m_body);
    return {static_cast<float>(position.x) * PHYSICS_PIXELS_PER_METER, static_cast<float>(position.y) * PHYSICS_PIXELS_PER_METER};
}

Vector2 RigidBody::get_velocity() const {
    const b2Vec2 velocity = b2Body_GetLinearVelocity(m_body);
    return {velocity.x * PHYSICS_PIXELS_PER_METER, velocity.y * PHYSICS_PIXELS_PER_METER};
}

Vector2 RigidBody::get_dimensions() const {
    const Rectangle bounds = get_bounding_box();
    return {bounds.width, bounds.height};
}

void RigidBody::set_dimensions(Vector2 dimensions) {
    const Rectangle bounds = get_bounding_box();
    if (!std::isfinite(dimensions.x) || !std::isfinite(dimensions.y) || dimensions.x <= 0.0F || dimensions.y <= 0.0F ||
        bounds.width <= 0.0F || bounds.height <= 0.0F) {
        return;
    }

    const Vector2 scale = {dimensions.x / bounds.width, dimensions.y / bounds.height};
    const Vector2 position = get_position();
    const b2Vec2 center = {
        (bounds.x + bounds.width * 0.5F - position.x) / PHYSICS_PIXELS_PER_METER,
        (bounds.y + bounds.height * 0.5F - position.y) / PHYSICS_PIXELS_PER_METER
    };

    const auto scale_point = [&center](b2Vec2 point, float factor) {
        return b2Vec2{center.x + (point.x - center.x) * factor, center.y + (point.y - center.y) * factor};
    };

    std::visit(
        [&](auto& shape) {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, b2Polygon>) {
                const float radius = shape.radius * std::min(scale.x, scale.y);
                b2Vec2 vertices[B2_MAX_POLYGON_VERTICES];

                // account for the rounded skin when scaling the polygon's outer bounds
                const float inner_width = bounds.width / PHYSICS_PIXELS_PER_METER - 2.0F * shape.radius;
                const float inner_height = bounds.height / PHYSICS_PIXELS_PER_METER - 2.0F * shape.radius;
                if (inner_width <= 0.0F || inner_height <= 0.0F) {
                    return;
                }

                const float scale_x = (dimensions.x / PHYSICS_PIXELS_PER_METER - 2.0F * radius) / inner_width;
                const float scale_y = (dimensions.y / PHYSICS_PIXELS_PER_METER - 2.0F * radius) / inner_height;

                // a non-positive scale would mirror the hull and still pass validation
                if (scale_x <= 0.0F || scale_y <= 0.0F) {
                    return;
                }

                for (int index = 0; index < shape.count; ++index) {
                    vertices[index] = {
                        center.x + (shape.vertices[index].x - center.x) * scale_x,
                        center.y + (shape.vertices[index].y - center.y) * scale_y
                    };
                }

                const b2Hull hull = b2ComputeHull(vertices, shape.count);
                if (hull.count < 3 || !b2ValidateHull(&hull)) {
                    return;
                }

                shape = b2MakePolygon(&hull, radius);
            } else {
                const float factor = std::abs(scale.x - 1.0F) >= std::abs(scale.y - 1.0F) ? scale.x : scale.y;
                if constexpr (std::is_same_v<T, b2Circle>) {
                    shape.center = scale_point(shape.center, factor);
                } else {
                    shape.center1 = scale_point(shape.center1, factor);
                    shape.center2 = scale_point(shape.center2, factor);
                }
                shape.radius *= factor;
            }
        },
        m_base_shape
    );

    update_shape();
}

Rectangle RigidBody::get_bounding_box() const {
    b2WorldTransform transform = b2Body_GetTransform(m_body);
    transform.q = b2Rot_identity;
    return to_rectangle(compute_aabb(m_base_shape, transform));
}

Rectangle RigidBody::get_world_bounding_box() const {
    return to_rectangle(b2Body_ComputeAABB(m_body));
}

b2BodyType RigidBody::get_body_type() const {
    return b2Body_GetType(m_body);
}

bool RigidBody::collision_enabled() const {
    return b2Body_IsEnabled(m_body);
}

void RigidBody::set_collision_enabled(bool enabled) {
    if (enabled == collision_enabled()) return;

    if (enabled) {
        b2Body_Enable(m_body);
    } else {
        b2Body_Disable(m_body);
    }

    m_grounded = false;
}

float RigidBody::get_gravity() const {
    return b2Body_GetGravityScale(m_body) * DEFAULT_GRAVITY;
}

bool RigidBody::is_sensor() const {
    return b2Shape_IsSensor(m_shape);
}

void RigidBody::set_position(float x, float y) {
    b2Body_SetTransform(m_body, {x / PHYSICS_PIXELS_PER_METER, y / PHYSICS_PIXELS_PER_METER}, b2Rot_identity);
    m_grounded = false;
}

float RigidBody::get_rotation() const {
    return m_rotation;
}

void RigidBody::set_rotation(float degrees) {
    m_rotation = degrees;
    update_shape();
}

void RigidBody::set_velocity(float x, float y) {
    b2Body_SetLinearVelocity(m_body, {x / PHYSICS_PIXELS_PER_METER, y / PHYSICS_PIXELS_PER_METER});
}

b2ShapeDef RigidBody::shape_definition(bool sensor) {
    b2ShapeDef definition = b2DefaultShapeDef();
    definition.userData = this;
    definition.material.friction = 0.0f;
    definition.material.restitution = 0.0f;
    definition.isSensor = sensor;
    definition.enableSensorEvents = true;

    if (!m_collidable) {
        definition.filter.maskBits = 0;
    }

    return definition;
}

void RigidBody::set_shape(const b2Polygon& polygon) {
    m_base_shape = polygon;
    update_shape();
}

void RigidBody::set_shape(const b2Circle& circle) {
    m_base_shape = circle;
    update_shape();
}

void RigidBody::set_shape(const b2Capsule& capsule) {
    m_base_shape = capsule;
    update_shape();
}

PhysicsShape RigidBody::rotated_shape() const {
    const float radians = m_rotation * (B2_PI / 180.0f);
    const b2Rot rotation = {std::cos(radians), std::sin(radians)};

    const b2AABB bounds = compute_aabb(m_base_shape, b2Transform_identity);
    const b2Vec2 pivot = {
        (bounds.lowerBound.x + bounds.upperBound.x) * 0.5f,
        (bounds.lowerBound.y + bounds.upperBound.y) * 0.5f,
    };

    const auto rotate = [&](b2Vec2 point) { return b2Add(pivot, b2RotateVector(rotation, b2Sub(point, pivot))); };

    return std::visit(
        [&](auto shape) -> PhysicsShape {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, b2Polygon>) {
                shape.centroid = rotate(shape.centroid);
                for (int index = 0; index < shape.count; ++index) {
                    shape.vertices[index] = rotate(shape.vertices[index]);
                    shape.normals[index] = b2RotateVector(rotation, shape.normals[index]);
                }
            } else if constexpr (std::is_same_v<T, b2Circle>) {
                shape.center = rotate(shape.center);
            } else {
                shape.center1 = rotate(shape.center1);
                shape.center2 = rotate(shape.center2);
            }

            return shape;
        },
        m_base_shape
    );
}

b2ShapeId RigidBody::create_shape(const b2ShapeDef& definition) const {
    return std::visit(
        [&](const auto& shape) {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, b2Polygon>) {
                return b2CreatePolygonShape(m_body, &definition, &shape);
            } else if constexpr (std::is_same_v<T, b2Circle>) {
                return b2CreateCircleShape(m_body, &definition, &shape);
            } else {
                return b2CreateCapsuleShape(m_body, &definition, &shape);
            }
        },
        rotated_shape()
    );
}

void RigidBody::update_shape() {
    std::visit(
        [this](const auto& shape) {
            using T = std::decay_t<decltype(shape)>;
            if constexpr (std::is_same_v<T, b2Polygon>) {
                b2Shape_SetPolygon(m_shape, &shape);
            } else if constexpr (std::is_same_v<T, b2Circle>) {
                b2Shape_SetCircle(m_shape, &shape);
            } else {
                b2Shape_SetCapsule(m_shape, &shape);
            }
        },
        rotated_shape()
    );

    // geometry setters do not recalculate body mass.
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void RigidBody::set_body_type(b2BodyType physics_type) {
    if (physics_type < b2_staticBody || physics_type >= b2_bodyTypeCount) {
        std::cerr << "[entity] warning: invalid body type, keeping current type\n";
        return;
    }

    b2Body_SetType(m_body, physics_type);
}

void RigidBody::set_gravity(float gravity) {
    b2Body_SetGravityScale(m_body, gravity / DEFAULT_GRAVITY);
}

void RigidBody::set_sensor(bool sensor) {
    if (sensor == is_sensor()) {
        return;
    }

    // the sensor flag is fixed at creation, so the shape must be recreated.
    const b2ShapeId replacement = create_shape(shape_definition(sensor));
    b2DestroyShape(m_shape, true);
    m_shape = replacement;
    m_grounded = false;
}

void RigidBody::serialize(Serializer& serializer) const {
    serializer.field("position", get_position());
    serializer.field("velocity", get_velocity());
    serializer.field("rotation", m_rotation);
    serializer.field("body_type", static_cast<int>(get_body_type()));
    serializer.field("gravity", get_gravity());
    serializer.field("is_sensor", is_sensor());
    serializer.field("collision_enabled", collision_enabled());

    serializer.field("shape", m_base_shape);
}

void RigidBody::deserialize(Deserializer& deserializer, const std::filesystem::path&) {
    PhysicsShape shape;
    if (deserializer.field("shape", shape)) {
        std::visit([this](const auto& value) { set_shape(value); }, shape);
    }

    int body_type = static_cast<int>(get_body_type());
    float gravity = get_gravity();
    float rotation = m_rotation;
    bool sensor = is_sensor();
    bool enabled = collision_enabled();
    Vector2 position = get_position();
    Vector2 velocity = get_velocity();

    deserializer.field("body_type", body_type);
    deserializer.field("gravity", gravity);
    deserializer.field("rotation", rotation);
    deserializer.field("is_sensor", sensor);
    deserializer.field("collision_enabled", enabled);
    deserializer.field("position", position);
    deserializer.field("velocity", velocity);

    set_body_type(static_cast<b2BodyType>(body_type));
    set_gravity(gravity);
    set_rotation(rotation);
    set_sensor(sensor);
    set_collision_enabled(enabled);
    set_position(position.x, position.y);
    set_velocity(velocity.x, velocity.y);
}
