#pragma once

#include "core/components/rigid-body.hpp"
#include "game/entity/entity.hpp"

class Platform : public Entity {
public:
    explicit Platform(World& world);
};
