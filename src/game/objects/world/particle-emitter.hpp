#pragma once

#include "game/objects/entities/entity.hpp"

class World;

class ParticleEmitter : public Entity {
public:
    ParticleEmitter(World& world, DashLevel& level);
};
