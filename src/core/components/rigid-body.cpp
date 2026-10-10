#include "rigid-body.hpp"
#include "core/object.hpp"

#include <iostream>
#include <stdexcept>

RigidBody::RigidBody(GameObject& object, World& world, b2BodyType physics_type, bool sensor, bool collidable)
    : Component(object, ComponentId::RigidBody), m_world(world), m_collidable(collidable) {
    b2BodyDef definition = b2DefaultBodyDef();
    definition.type = physics_type;
    definition.motionLocks.angularZ = true;
    definition.enableSleep = false;
    definition.enableContactRecycling = false;
    m_body = b2CreateBody(m_world.id(), &definition);

    const b2ShapeDef shape_def = shape_definition(sensor);
    const b2Polygon box = make_box_shape(64.0f, 64.0f);
    m_shape = b2CreatePolygonShape(m_body, &shape_def, &box);

    m_world.add(*this);
}

RigidBody::~RigidBody() {
    b2DestroyBody(m_body);
    m_world.remove(*this);
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

Rectangle RigidBody::get_bounding_box() const {
    const b2WorldTransform transform = b2Body_GetTransform(m_body);
    b2AABB bounds;

    // box2d's cached aabb includes speculative collision padding.
    switch (b2Shape_GetType(m_shape)) {
        case b2_polygonShape: {
            const b2Polygon polygon = b2Shape_GetPolygon(m_shape);
            bounds = b2ComputePolygonAABB(&polygon, transform);
            break;
        }
        case b2_circleShape: {
            const b2Circle circle = b2Shape_GetCircle(m_shape);
            bounds = b2ComputeCircleAABB(&circle, transform);
            break;
        }
        case b2_capsuleShape: {
            const b2Capsule capsule = b2Shape_GetCapsule(m_shape);
            bounds = b2ComputeCapsuleAABB(&capsule, transform);
            break;
        }
        default:
            throw std::logic_error("unsupported entity shape");
    }

    // convert the tight box2d bounds from meters to raylib pixels.
    return {
        bounds.lowerBound.x * PHYSICS_PIXELS_PER_METER, bounds.lowerBound.y * PHYSICS_PIXELS_PER_METER,
        (bounds.upperBound.x - bounds.lowerBound.x) * PHYSICS_PIXELS_PER_METER,
        (bounds.upperBound.y - bounds.lowerBound.y) * PHYSICS_PIXELS_PER_METER
    };
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
    b2Shape_SetPolygon(m_shape, &polygon);

    // geometry setters do not recalculate body mass.
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void RigidBody::set_shape(const b2Circle& circle) {
    b2Shape_SetCircle(m_shape, &circle);
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void RigidBody::set_shape(const b2Capsule& capsule) {
    b2Shape_SetCapsule(m_shape, &capsule);
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void RigidBody::set_body_type(b2BodyType physics_type) {
    if (physics_type < b2_staticBody || physics_type >= b2_bodyTypeCount) {
        std::cerr << "[entity] warning: invalid body type\nkeeping current type\n";
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

    // changing the sensor flag requires recreating the shape.
    const b2ShapeDef definition = shape_definition(sensor);
    b2ShapeId replacement;

    switch (b2Shape_GetType(m_shape)) {
        case b2_polygonShape: {
            const b2Polygon polygon = b2Shape_GetPolygon(m_shape);
            replacement = b2CreatePolygonShape(m_body, &definition, &polygon);
            break;
        }
        case b2_circleShape: {
            const b2Circle circle = b2Shape_GetCircle(m_shape);
            replacement = b2CreateCircleShape(m_body, &definition, &circle);
            break;
        }
        case b2_capsuleShape: {
            const b2Capsule capsule = b2Shape_GetCapsule(m_shape);
            replacement = b2CreateCapsuleShape(m_body, &definition, &capsule);
            break;
        }
        default:
            throw std::logic_error("unsupported entity shape");
    }

    b2DestroyShape(m_shape, true);
    m_shape = replacement;
    m_grounded = false;
}

void RigidBody::serialize(Serializer& serializer) const {
    serializer.field("position", get_position());
    serializer.field("body_type", static_cast<int>(get_body_type()));
    serializer.field("gravity", get_gravity());
    serializer.field("is_sensor", is_sensor());
    serializer.field("collision_enabled", collision_enabled());

    PhysicsShape shape;
    switch (b2Shape_GetType(m_shape)) {
        case b2_polygonShape:
            shape = b2Shape_GetPolygon(m_shape);
            break;
        case b2_circleShape:
            shape = b2Shape_GetCircle(m_shape);
            break;
        case b2_capsuleShape:
            shape = b2Shape_GetCapsule(m_shape);
            break;
        default:
            throw std::logic_error("unsupported entity shape");
    }

    serializer.field("shape", shape);
}

void RigidBody::deserialize(Deserializer& deserializer, const std::filesystem::path&) {
    PhysicsShape shape;
    if (deserializer.field("shape", shape)) {
        std::visit([this](const auto& value) { set_shape(value); }, shape);
    }

    int body_type = static_cast<int>(get_body_type());
    float gravity = get_gravity();
    bool sensor = is_sensor();
    bool enabled = collision_enabled();
    Vector2 position = get_position();

    deserializer.field("body_type", body_type);
    deserializer.field("gravity", gravity);
    deserializer.field("is_sensor", sensor);
    deserializer.field("collision_enabled", enabled);
    deserializer.field("position", position);

    set_body_type(static_cast<b2BodyType>(body_type));
    set_gravity(gravity);
    set_sensor(sensor);
    set_collision_enabled(enabled);
    set_position(position.x, position.y);
}
