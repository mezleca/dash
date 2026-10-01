#pragma once

#include <box2d/box2d.h>
#include <vector>

constexpr float DEFAULT_GRAVITY = 5332.5f;
constexpr float FALL_MAX_SPEED = 2237.0f;
constexpr float DEFAULT_HORIZONTAL_DAMPING = 12.0f;
constexpr float PHYSICS_PIXELS_PER_METER = 64.0f;

class Entity;

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

    const std::vector<Entity*>& entities() const {
        return m_entities;
    }

    void add(Entity& entity);
    void remove(Entity& entity);

    void step(float timestep);

private:
    b2WorldId m_id;
    std::vector<Entity*> m_entities;
    std::vector<b2ContactData> m_contacts;
};
