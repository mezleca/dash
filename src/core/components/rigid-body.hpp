#pragma once

#include "component.hpp"
#include "core/physics/world.hpp"
#include "core/physics/shape.hpp"

class RigidBody : public Component {
public:
    RigidBody(
        GameObject& object, World& world, b2BodyType body_type = b2_staticBody, bool sensor = false, bool collidable = true
    );
    ~RigidBody() override;
    RigidBody(const RigidBody&) = delete;
    RigidBody& operator=(const RigidBody&) = delete;
    RigidBody(RigidBody&&) = delete;
    RigidBody& operator=(RigidBody&&) = delete;

    bool collision_enabled() const;
    void set_collision_enabled(bool enabled);

    bool is_grounded() const {
        return m_grounded;
    }

    Vector2 get_position() const;
    void set_position(float x, float y);

    Vector2 get_velocity() const;
    void set_velocity(float x, float y);

    Rectangle get_bounding_box() const;
    Vector2 get_dimensions() const;
    b2ShapeId get_shape() const {
        return m_shape;
    }

    void set_shape(const b2Polygon& polygon);
    void set_shape(const b2Circle& circle);
    void set_shape(const b2Capsule& capsule);

    b2BodyType get_body_type() const;
    void set_body_type(b2BodyType type);

    float get_gravity() const;
    void set_gravity(float gravity);

    bool is_sensor() const;
    void set_sensor(bool sensor);

    World& world() const {
        return m_world;
    }

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

private:
    b2ShapeDef shape_definition(bool sensor);

    friend class World;

    World& m_world;
    b2BodyId m_body = {};
    b2ShapeId m_shape = {};
    bool m_grounded = false;
    bool m_collidable = true;
};
