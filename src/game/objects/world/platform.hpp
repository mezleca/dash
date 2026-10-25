#pragma once

#include "core/components/rigid-body.hpp"
#include "game/objects/entities/entity.hpp"

class Platform : public Entity {
public:
    explicit Platform(World& world);
};
