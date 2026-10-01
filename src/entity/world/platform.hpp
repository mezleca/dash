#pragma once

#include "../entity.hpp"

class Platform : public Entity {
public:
    explicit Platform(World& world);

    void render() override;
};
