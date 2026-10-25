#pragma once

#include "game/objects/entities/entity.hpp"

class World;

class Start : public Entity {
public:
    Start(World& world, DashLevel& level);
};
