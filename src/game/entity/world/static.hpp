#pragma once

#include "core/components/rigid-body.hpp"
#include "game/entity/entity.hpp"

class StaticTexture : public Entity {
public:
    explicit StaticTexture(World& world, bool fill_viewport = false);
};
