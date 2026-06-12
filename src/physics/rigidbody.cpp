#include "rigidbody.hpp"
#include "../game/game.hpp"

#include <algorithm>

static bool aabb(const GameObject& a, const GameObject& b) {
    return !(a.position.x + a.dimensions.x <= b.position.x || a.position.x >= b.position.x + b.dimensions.x ||
             a.position.y + a.dimensions.y <= b.position.y || a.position.y >= b.position.y + b.dimensions.y);
}

RigidBody::RigidBody(GameObject* _obj) : obj(_obj) {
    m_gravity = DEFAULT_GRAVITY;
    m_horizontal_damp = DEFAULT_HORIZONTAL_DAMPING;
}

void RigidBody::simulate() {
    if (is_static) return;

    const float horizontal_damping = std::max(0.0f, 1.0f - m_horizontal_damp * game.m_fixed_frametime);

    obj->velocity.x *= horizontal_damping;
    obj->velocity.y += m_gravity * game.m_fixed_frametime;
    obj->velocity.y = std::min(obj->velocity.y, FALL_MAX_SPEED);

    // resolve x
    obj->position.x += obj->velocity.x * game.m_fixed_frametime;

    for (const auto& object : game.m_objects) {
        if (object == this->obj) continue;
        if (!object->visible) continue;
        if (!aabb(*obj, *object)) continue;

        if (!object->rb->is_trigger) {
            float overlap_left = (object->position.x + object->dimensions.x) - obj->position.x;
            float overlap_right = (obj->position.x + obj->dimensions.x) - object->position.x;

            if (overlap_left < overlap_right) {
                obj->position.x += overlap_left;
            } else {
                obj->position.x -= overlap_right;
            }

            obj->velocity.x = 0.0f;
        }

        if (on_hit) {
            on_hit(object);
        }
    }

    grounded = false;

    // resolve y
    obj->position.y += obj->velocity.y * game.m_fixed_frametime;

    for (const auto& object : game.m_objects) {
        if (object == this->obj) continue;
        if (!object->visible) continue;
        if (!aabb(*obj, *object)) continue;

        if (!object->rb->is_trigger) {
            float overlap_top = (object->position.y + object->dimensions.y) - obj->position.y;
            float overlap_bot = (obj->position.y + obj->dimensions.y) - object->position.y;

            if (overlap_bot < overlap_top) {
                obj->position.y -= overlap_bot;
                obj->velocity.y = 0.0f;
                grounded = true;
            } else {
                obj->position.y += overlap_top;
                obj->velocity.y = 0.0f;
            }
        }

        if (on_hit) {
            on_hit(object);
        }
    }
}
