#pragma once

#include "../entity.hpp"
#include "../../level/behaviour.hpp"

#include <functional>
#include <memory>

class DashLevel;

class Trigger : public Entity {
public:
    Trigger(World& world, DashLevel& level);

    void set_action(std::function<std::unique_ptr<Behaviour>(Entity&)> action);
    void on_sensor(Entity& visitor) override;

private:
    DashLevel& m_level;
    std::function<std::unique_ptr<Behaviour>(Entity&)> m_action;
};
