#include "entity.hpp"

#include <iostream>
#include <stdexcept>

Entity::Entity(World& world, ObjectType object_type, b2BodyType physics_type) : GameObject(object_type), m_world(world) {
    b2BodyDef definition = b2DefaultBodyDef();
    definition.type = physics_type;
    definition.motionLocks.angularZ = true;
    definition.enableSleep = false;
    definition.enableContactRecycling = false;
    definition.userData = this;

    m_body = b2CreateBody(m_world.id(), &definition);

    const b2ShapeDef shape_def = shape_definition(type() == ObjectType::END || type() == ObjectType::TRIGGER);
    const b2Polygon box = make_box_shape(64.0f, 64.0f);
    m_shape = b2CreatePolygonShape(m_body, &shape_def, &box);

    m_world.add(*this);
}

Entity::~Entity() {
    b2DestroyBody(m_body);
    m_world.remove(*this);
}

Vector2 Entity::get_position() const {
    const b2Pos position = b2Body_GetPosition(m_body);
    return {static_cast<float>(position.x) * PHYSICS_PIXELS_PER_METER, static_cast<float>(position.y) * PHYSICS_PIXELS_PER_METER};
}

Vector2 Entity::get_velocity() const {
    const b2Vec2 velocity = b2Body_GetLinearVelocity(m_body);
    return {velocity.x * PHYSICS_PIXELS_PER_METER, velocity.y * PHYSICS_PIXELS_PER_METER};
}

Vector2 Entity::get_dimensions() const {
    const Rectangle bounds = get_bounding_box();
    return {bounds.width, bounds.height};
}

Rectangle Entity::get_bounding_box() const {
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

b2BodyType Entity::get_body_type() const {
    return b2Body_GetType(m_body);
}

bool Entity::collision_enabled() const {
    return b2Body_IsEnabled(m_body);
}

void Entity::set_collision_enabled(bool enabled) {
    if (enabled == collision_enabled()) return;

    if (enabled) {
        b2Body_Enable(m_body);
    } else {
        b2Body_Disable(m_body);
    }

    m_grounded = false;
}

float Entity::get_gravity() const {
    return b2Body_GetGravityScale(m_body) * DEFAULT_GRAVITY;
}

bool Entity::is_trigger() const {
    return b2Shape_IsSensor(m_shape);
}

void Entity::set_position(float x, float y) {
    b2Body_SetTransform(m_body, {x / PHYSICS_PIXELS_PER_METER, y / PHYSICS_PIXELS_PER_METER}, b2Rot_identity);
    m_previous_position = {x, y};
    m_grounded = false;
}

void Entity::set_velocity(float x, float y) {
    b2Body_SetLinearVelocity(m_body, {x / PHYSICS_PIXELS_PER_METER, y / PHYSICS_PIXELS_PER_METER});
}

b2ShapeDef Entity::shape_definition(bool trigger) const {
    b2ShapeDef definition = b2DefaultShapeDef();
    definition.material.friction = 0.0f;
    definition.material.restitution = 0.0f;
    definition.isSensor = trigger;
    definition.enableSensorEvents = true;

    if (type() == ObjectType::STATIC_TEXTURE) {
        definition.filter.maskBits = 0;
    }

    return definition;
}

void Entity::set_shape(const b2Polygon& polygon) {
    b2Shape_SetPolygon(m_shape, &polygon);

    // geometry setters do not recalculate body mass.
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void Entity::set_shape(const b2Circle& circle) {
    b2Shape_SetCircle(m_shape, &circle);
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void Entity::set_shape(const b2Capsule& capsule) {
    b2Shape_SetCapsule(m_shape, &capsule);
    b2Body_UpdateMassFromShapes(m_body);
    m_grounded = false;
}

void Entity::set_body_type(b2BodyType physics_type) {
    if (physics_type < b2_staticBody || physics_type >= b2_bodyTypeCount) {
        std::cerr << "[entity] warning: invalid body type\nkeeping current type\n";
        return;
    }

    b2Body_SetType(m_body, physics_type);
}

void Entity::set_gravity(float gravity) {
    b2Body_SetGravityScale(m_body, gravity / DEFAULT_GRAVITY);
}

void Entity::set_trigger(bool trigger) {
    if (trigger == is_trigger()) {
        return;
    }

    // changing the sensor flag requires recreating the shape.
    const b2ShapeDef definition = shape_definition(trigger);
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

nlohmann::json Entity::serialize() const {
    auto data = GameObject::serialize();
    data.update(
        {{"position", get_position()},
         {"body_type", get_body_type()},
         {"gravity", get_gravity()},
         {"is_trigger", is_trigger()},
         {"collision_enabled", collision_enabled()}}
    );

    switch (b2Shape_GetType(m_shape)) {
        case b2_polygonShape:
            data["shape"] = b2Shape_GetPolygon(m_shape);
            break;
        case b2_circleShape:
            data["shape"] = b2Shape_GetCircle(m_shape);
            break;
        case b2_capsuleShape:
            data["shape"] = b2Shape_GetCapsule(m_shape);
            break;
        default:
            throw std::logic_error("unsupported entity shape");
    }

    return data;
}

void Entity::deserialize(const nlohmann::json& data, const std::filesystem::path& directory) {
    GameObject::deserialize(data, directory);

    // restore geometry before body and sensor settings that act on the shape.
    const auto& shape = data.at("shape");
    const std::string shape_type = shape.at("type").get<std::string>();

    if (shape_type == "polygon") {
        set_shape(shape.get<b2Polygon>());
    } else if (shape_type == "circle") {
        set_shape(shape.get<b2Circle>());
    } else if (shape_type == "capsule") {
        set_shape(shape.get<b2Capsule>());
    } else {
        throw std::invalid_argument("unsupported shape type: " + shape_type);
    }

    set_body_type(data.value("body_type", get_body_type()));
    set_gravity(data.value("gravity", get_gravity()));
    set_trigger(data.value("is_trigger", is_trigger()));
    set_collision_enabled(data.value("collision_enabled", collision_enabled()));

    const Vector2 start = data.value("position", Vector2{});
    set_position(start.x, start.y);
}
