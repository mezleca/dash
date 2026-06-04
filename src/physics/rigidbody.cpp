#include "rigidbody.hpp"
#include "../entity/entity.hpp"

struct CollisionResult {
    bool hit;
    float normal_x;
    float normal_y;
    float depth_x;
    float depth_y;
};

static CollisionResult resolve_aabb(const GameObject& a, const GameObject& b) {
    float overlap_left = (b.position.x + b.dimensions.x) - a.position.x;
    float overlap_right = (a.position.x + a.dimensions.x) - b.position.x;
    float overlap_top = (b.position.y + b.dimensions.y) - a.position.y;
    float overlap_bot = (a.position.y + a.dimensions.y) - b.position.y;

    bool hit = overlap_left > 0 && overlap_right > 0 && overlap_top > 0 && overlap_bot > 0;
    if (!hit) return {false};

    // pick the axis with minimum penetration
    float min_x = overlap_left < overlap_right ? overlap_left : overlap_right;
    float min_y = overlap_top < overlap_bot ? overlap_top : overlap_bot;

    float nx = 0.0f, ny = 0.0f;
    if (min_x < min_y) {
        nx = overlap_left < overlap_right ? 1.0f : -1.0f;
    } else {
        ny = overlap_bot < overlap_top ? 1.0f : -1.0f;
    }

    return {true, nx, ny, min_x, min_y};
}

void RigidBody::simulate() {
    if (is_static) return;

    float gravity = GRAVITY;

    if (entity->velocity.y > 0.0f) {
        gravity *= FALL_GRAVITY_MULT;
    }

    entity->velocity.y += gravity * game.fixed_timestep;

    for (const auto& object : game.m_objects) {
        if (object == this->entity) {
            continue;
        }

        grounded = false;

        // TODO: swept aabb would prob be better but i have no idea how it works so
        // this will have to do it
        auto col = resolve_aabb(*entity, *object);
        if (!col.hit) continue;

        if (col.normal_y > 0.0f) {
            // std::cout << "hit top of object" << "\n";
            entity->velocity.y = 0.0f;
            entity->position.y -= col.depth_y;
            grounded = true;
        } else if (col.normal_y < 0.0f) {
            // std::cout << "hit ceiling" << "\n";
            entity->velocity.y = 0.0f;
            entity->position.y += col.depth_y;
        } else if (col.normal_x != 0.0f) {
            // std::cout << "hit side wall" << "\n";
            entity->velocity.x = 0.0f;
            entity->position.x -= col.normal_x * col.depth_x;
        }
    }
}
