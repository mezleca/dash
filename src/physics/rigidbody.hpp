#pragma once

#include <functional>

// TODO: move to the struct?
constexpr float GRAVITY = 5332.5f;
constexpr float FALL_MAX_SPEED = 2237.0f;
constexpr float FRICTION = 0.65f;

struct GameObject;

struct RigidBody {
  public:
    explicit RigidBody(GameObject* _obj) : obj(_obj) {
    }

    std::function<void(GameObject*)> on_hit = nullptr;

    GameObject* obj;
    GameObject* last_hit;

    bool is_static = false;
    bool grounded = false;

    void simulate();
};
