#pragma once

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"

class Platform : public GameObject {
public:
    explicit Platform(World& world);
};
