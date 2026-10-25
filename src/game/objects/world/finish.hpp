#pragma once

#include "game/objects/entities/entity.hpp"

class DashLevel;
class World;

class Finish : public Entity {
public:
    Finish(World& world, DashLevel& level);

    void on_sensor(GameObject& visitor) override;

private:
    DashLevel& m_level;
};
