#pragma once

#include <box2d/box2d.h>
#include <vector>

constexpr float DEFAULT_GRAVITY = 5332.5f;
constexpr float FALL_MAX_SPEED = 2237.0f;
constexpr float PHYSICS_PIXELS_PER_METER = 64.0f;

class RigidBody;

class World {
public:
    World();
    ~World();
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

    b2WorldId id() const {
        return m_id;
    }

    const std::vector<RigidBody*>& bodies() const {
        return m_bodies;
    }

    void add(RigidBody& body);
    void remove(RigidBody& body);

    void step(float timestep);

private:
    void update_contacts();
    void dispatch_sensors();

    b2WorldId m_id;
    std::vector<RigidBody*> m_bodies;
    std::vector<b2ContactData> m_contacts;
};
