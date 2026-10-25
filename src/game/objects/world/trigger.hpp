#pragma once

#include "game/objects/entities/entity.hpp"

class DashLevel;
class World;

class Trigger : public Entity {
public:
    Trigger(World& world, DashLevel& level);

    void on_sensor(GameObject&) override;
    void reset_behaviours() override;
};
