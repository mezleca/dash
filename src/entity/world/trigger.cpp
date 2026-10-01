#include "trigger.hpp"
#include "../../level/level.hpp"

#include <utility>

Trigger::Trigger(World& world, DashLevel& level) : Entity(world, ObjectType::TRIGGER), m_level(level) {}

void Trigger::set_action(std::function<std::unique_ptr<Behaviour>(Entity&)> action) {
    m_action = std::move(action);
}

void Trigger::on_sensor(Entity& visitor) {
    if (!m_action) return;

    m_level.add_behaviour(m_action(visitor));
}
