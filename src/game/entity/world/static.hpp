#pragma once

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"

class StaticTexture : public GameObject {
public:
    explicit StaticTexture(World& world, bool fill_viewport = false);
};
