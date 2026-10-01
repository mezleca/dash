#pragma once

#include "../game/object.hpp"
#include "../physics/world.hpp"
#include "../physics/shape.hpp"

#include <filesystem>

class Entity : public GameObject {
public:
    explicit Entity(World& world, ObjectType type, b2BodyType body_type = b2_staticBody);
    ~Entity() override;

    float horizontal_damping = DEFAULT_HORIZONTAL_DAMPING;
    bool collision_enabled = true;
    bool grounded = false;

    Vector2 get_position() const;
    void set_position(float x, float y);

    Vector2 get_previous_position() const {
        return m_previous_position;
    }

    Vector2 get_velocity() const;
    void set_velocity(float x, float y);

    Rectangle get_bounding_box() const;
    Vector2 get_dimensions() const;

    b2ShapeId get_shape() const {
        return m_shape;
    }

    // NOTE: box2d uses meters internally but we serialize as pixels
    void set_shape(const b2Polygon& polygon);
    void set_shape(const b2Circle& circle);
    void set_shape(const b2Capsule& capsule);

    b2BodyType get_body_type() const;
    float get_gravity() const;
    bool is_trigger() const;

    void set_body_type(b2BodyType type);
    void set_gravity(float gravity);
    void set_trigger(bool trigger);

    World& world() const {
        return m_world;
    }

    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& data, const std::filesystem::path& directory) override;

    virtual void on_contact(Entity&, Vector2) {}
    virtual void on_sensor(Entity&) {}

private:
    friend class World;

    World& m_world;
    b2BodyId m_body = {};
    b2ShapeId m_shape = {};
    Vector2 m_previous_position = {};

    void prepare_physics(float timestep);
    b2ShapeDef shape_definition(bool trigger) const;
};
