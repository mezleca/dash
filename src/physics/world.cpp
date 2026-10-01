#include "world.hpp"
#include "../entity/entity.hpp"

World::World() {
    b2WorldDef definition = b2DefaultWorldDef();
    definition.gravity = {0.0f, DEFAULT_GRAVITY / PHYSICS_PIXELS_PER_METER};
    definition.maximumLinearSpeed = 1000.0f;
    definition.enableSleep = false;

    m_id = b2CreateWorld(&definition);
}

World::~World() {
    b2DestroyWorld(m_id);
}

void World::add(Entity& entity) {
    m_entities.push_back(&entity);
}

void World::remove(Entity& entity) {
    std::erase(m_entities, &entity);
}

void World::step(float timestep) {
    for (Entity* entity : m_entities) {
        entity->prepare_physics(timestep);
        entity->grounded = false;
    }

    // one substep keeps the old full-tick gravity integration.
    b2World_Step(m_id, timestep, 1);

    for (Entity* entity : m_entities) {
        if (entity->get_body_type() != b2_dynamicBody || !entity->visible || !entity->collision_enabled) {
            continue;
        }

        m_contacts.resize(static_cast<size_t>(b2Body_GetContactCapacity(entity->m_body)));
        const int count = b2Body_GetContactData(entity->m_body, m_contacts.data(), static_cast<int>(m_contacts.size()));

        for (int i = 0; i < count; ++i) {
            const b2ContactData& contact = m_contacts[static_cast<size_t>(i)];
            bool touching = false;

            // ignore speculative contacts that received no collision impulse.
            for (int point = 0; point < contact.manifold.pointCount; ++point) {
                const b2ManifoldPoint& manifold_point = contact.manifold.points[point];
                touching |= manifold_point.separation <= 0.005f || manifold_point.totalNormalImpulse > 0.0f;

                if (touching) {
                    break;
                }
            }

            if (!touching) {
                continue;
            }

            const bool is_a = B2_ID_EQUALS(b2Shape_GetBody(contact.shapeIdA), entity->m_body);
            const b2ShapeId other_shape = is_a ? contact.shapeIdB : contact.shapeIdA;
            auto* other = static_cast<Entity*>(b2Body_GetUserData(b2Shape_GetBody(other_shape)));

            // point the normal from this entity toward the other collider.
            const float sign = is_a ? 1.0f : -1.0f;
            const Vector2 normal = {contact.manifold.normal.x * sign, contact.manifold.normal.y * sign};

            const Vector2 velocity = entity->get_velocity();
            const Vector2 other_velocity = other->get_velocity();
            const float approach_speed = (velocity.x - other_velocity.x) * normal.x + (velocity.y - other_velocity.y) * normal.y;

            // ignore contacts the bodies are already moving away from, such as after a jump.
            if (approach_speed < -1.0f) {
                continue;
            }

            entity->grounded |= normal.y > 0.5f;
            entity->on_contact(*other, normal);
        }
    }

    // notify both entities when a sensor overlap begins.
    const b2SensorEvents sensors = b2World_GetSensorEvents(m_id);

    for (int i = 0; i < sensors.beginCount; ++i) {
        const b2SensorBeginTouchEvent& event = sensors.beginEvents[i];

        auto* sensor = static_cast<Entity*>(b2Body_GetUserData(b2Shape_GetBody(event.sensorShapeId)));

        auto* visitor = static_cast<Entity*>(b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId)));

        visitor->on_sensor(*sensor);

        sensor->on_sensor(*visitor);
    }
}
