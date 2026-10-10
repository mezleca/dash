#include "trigger.hpp"
#include "game/level/level.hpp"

#include <utility>

Trigger::Trigger(World& world, DashLevel& level) : m_level(level) {
    add_component<RigidBody>(world, b2_staticBody, true);
}

void Trigger::set_action(std::function<std::unique_ptr<Behaviour>(GameObject&)> action) {
    m_action = std::move(action);
}

void Trigger::on_sensor(GameObject& visitor) {
    if (!m_action) return;

    m_level.add_behaviour(m_action(visitor));
}
