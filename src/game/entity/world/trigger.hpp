#pragma once

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"
#include "game/level/behaviour.hpp"

#include <functional>
#include <memory>

class DashLevel;

class Trigger : public GameObject {
public:
    Trigger(World& world, DashLevel& level);

    void set_action(std::function<std::unique_ptr<Behaviour>(GameObject&)> action);
    void on_sensor(GameObject& visitor) override;

private:
    DashLevel& m_level;
    std::function<std::unique_ptr<Behaviour>(GameObject&)> m_action;
};
