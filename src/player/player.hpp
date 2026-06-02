#pragma once

#include "raylib.h"
#include <cstdint>

struct Entity;

struct GameObject {
    explicit GameObject();
    ~GameObject();

    Vector2 position;
    uint32_t id = 0;
};

struct RigidBody {
  public:
    explicit RigidBody(Entity* ent) : entity(ent) {
    }

    Entity* entity;
    bool is_static = false;
    bool grounded = false;

    void simulate();

  private:
};

enum EntityState : int { GROUND, JUMP, FREE };

struct Entity : public GameObject {
  public:
    explicit Entity();

    Vector2 velocity;
    RigidBody* rb;
    float last_x, last_y;
    EntityState state;

  private:
};

struct Player : public Entity {
    int width, height;
    bool can_jump = false;

    void movement();
    void jump();
    void render();
};
