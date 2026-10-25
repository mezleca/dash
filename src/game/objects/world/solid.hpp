#pragma once

#include "game/objects/entities/entity.hpp"

class World;
enum class SpriteType : uint8_t;

class Solid : public Entity {
public:
    explicit Solid(World& world);
    bool set_shape(SpriteType type);
};
