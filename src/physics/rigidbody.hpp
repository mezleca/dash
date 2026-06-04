#pragma once

constexpr float GRAVITY = 600.0f;
constexpr float FALL_GRAVITY_MULT = 1.45f;

struct Entity;

struct RigidBody {
  public:
    explicit RigidBody(Entity* ent) : entity(ent) {
    }

    Entity* entity;
    bool is_static = false;
    bool grounded = false;

    void simulate();
};
