#pragma once

#include "../../game/object.hpp"

struct Platform : public GameObject {
    explicit Platform(float width, float height);

    void render() override;
};
