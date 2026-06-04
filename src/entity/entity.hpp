#pragma once

#include "../game/game.hpp"

struct RigidBody;

enum EntityState : int { GROUND, JUMP, FREE };

struct Entity : public GameObject {
  public:
    explicit Entity();

    Vector2 velocity;
    RigidBody* rb;
    Vector2 previous_position;
    EntityState state;

  private:
};
