#pragma once

#include <functional>

constexpr float GRAVITY = 5332.5f;
constexpr float FALL_MAX_SPEED = 2237.0f;
constexpr float HORIZONTAL_DAMPING = 12.0f;

struct GameObject;

struct RigidBody {
  public:
    explicit RigidBody(GameObject* _obj) : obj(_obj) {
    }

    std::function<void(GameObject*)> on_hit = nullptr;

    GameObject* obj;
    GameObject* last_hit;

    bool is_static = false;
    bool is_trigger = false;
    bool grounded = false;

    void simulate();
};
