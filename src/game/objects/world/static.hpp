#pragma once

#include "core/components/rigid-body.hpp"
#include "game/objects/entities/entity.hpp"

class StaticTexture : public Entity {
public:
    explicit StaticTexture(World& world);
};
