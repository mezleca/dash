#include "rigidbody.hpp"
#include "../entity/entity.hpp"

#include <algorithm>
#include <iostream>

static bool aabb(const GameObject& a, const GameObject& b) {
    return !(
        a.position.x + a.dimensions.x <= b.position.x ||
        a.position.x >= b.position.x + b.dimensions.x ||
        a.position.y + a.dimensions.y <= b.position.y ||
        a.position.y >= b.position.y + b.dimensions.y
    );
}

void RigidBody::simulate() {
    if (is_static) return;

    entity->velocity.x *= FRICTION * game.fixed_timestep;
    entity->velocity.y += GRAVITY * game.fixed_timestep; // TODO: clamp

    // resolve x
    entity->position.x += entity->velocity.x;

    for (const auto& object : game.m_objects) {
        if (object == this->entity) continue;
        if (!aabb(*entity, *object)) continue;

        float overlap_left  = (object->position.x + object->dimensions.x) - entity->position.x;
        float overlap_right = (entity->position.x + entity->dimensions.x) - object->position.x;

        if (overlap_left < overlap_right) {
            entity->position.x += overlap_left;
        } else {
            entity->position.x -= overlap_right;
        }

        entity->velocity.x = 0.0f;

        if (on_hit) {
            on_hit(object);
        }
    }

    grounded = false;

    // resolve y
    entity->position.y += entity->velocity.y * game.fixed_timestep;

    for (const auto& object : game.m_objects) {
        if (object == this->entity) continue;
        if (!aabb(*entity, *object)) continue;

        float overlap_top = (object->position.y + object->dimensions.y) - entity->position.y;
        float overlap_bot = (entity->position.y + entity->dimensions.y) - object->position.y;

        if (overlap_bot < overlap_top) {
            entity->position.y -= overlap_bot;
            entity->velocity.y = 0.0f;
            grounded = true;
        } else {
            entity->position.y += overlap_top;
            entity->velocity.y = 0.0f;
        }

        if (on_hit) {
            on_hit(object);
        }
    }
}
