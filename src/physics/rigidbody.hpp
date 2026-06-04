#pragma once

#include <functional>

// TODO: move to the struct?
constexpr float GRAVITY = 850.0f;
constexpr float FRICTION = 0.65f;

struct Entity;
struct GameObject;

struct RigidBody {
  public:
    explicit RigidBody(Entity* ent) : entity(ent) {
    }

    std::function<void(GameObject*)> on_hit = nullptr;

    Entity* entity;
    GameObject* last_hit_obj;
    bool is_static = false;
    bool grounded = false;

    void simulate();
};
