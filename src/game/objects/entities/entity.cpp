#include "entity.hpp"
#include "game/behaviours/pulse.hpp"
#include "game/behaviours/camera-focus.hpp"
#include "game/behaviours/player-type.hpp"
#include "game/level/level.hpp"
#include "core/components/sprite.hpp"

#include <algorithm>
#include <stdexcept>
#include <limits>

static int next_entity_id = 1;

Entity::Entity(EntityType type) : m_type(type) {
    assign_new_id();
}

void Entity::assign_new_id() {
    if (next_entity_id == std::numeric_limits<int>::max()) throw std::overflow_error("entity ids exhausted");
    m_id = next_entity_id++;
}

Behaviour& Entity::add_behaviour(BehaviourType type) {
    if (type == BehaviourType::Pulse) {
        auto* sprite = get_component<Sprite>();
        if (sprite != nullptr) {
            return *m_behaviours.emplace_back(std::make_unique<Pulse>(*sprite));
        }
    } else if (type == BehaviourType::CameraFocus && m_level != nullptr) {
        return *m_behaviours.emplace_back(std::make_unique<CameraFocus>(*this, *m_level));
    } else if (type == BehaviourType::PlayerType && m_level != nullptr) {
        return *m_behaviours.emplace_back(std::make_unique<SetPlayerType>(*m_level));
    }

    throw std::invalid_argument("unsupported behaviour for this object");
}

void Entity::remove_behaviour(Behaviour& behaviour) {
    behaviour.reset();
    std::erase_if(m_behaviours, [&behaviour](const auto& entry) { return entry.get() == &behaviour; });
}

void Entity::update_behaviours(float frametime) {
    if (!m_behaviours_active) return;

    const auto count = m_behaviours.size();
    for (std::size_t index = 0; index < count; ++index) {
        if (!m_behaviours[index]->is_finished()) {
            m_behaviours[index]->update(frametime);
        }
    }
}

void Entity::update(float frametime) {
    if (m_level != nullptr && m_level->state() == LevelState::PAUSED) return;
    GameObject::update(frametime);
}

void Entity::reset_behaviours() {
    m_behaviours_active = true;

    for (auto& behaviour : m_behaviours) {
        behaviour->reset();
    }
}

void Entity::serialize(Serializer& serializer) const {
    GameObject::serialize(serializer);

    serializer.field("id", m_id);
    serializer.begin_array("behaviours");

    for (const auto& behaviour : m_behaviours) {
        const auto type = behaviour->type();
        if (type == BehaviourType::None) {
            continue;
        }

        serializer.begin_element();
        serializer.field("type", static_cast<int>(type));
        behaviour->serialize(serializer);
        serializer.end_element();
    }

    serializer.end_array();
}

void Entity::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    GameObject::deserialize(deserializer, directory);
    deserializer.field("id", m_id);
    next_entity_id = std::max(next_entity_id, m_id + 1);

    m_behaviours.clear();

    const auto count = deserializer.array_size("behaviours");
    for (std::size_t index = 0; index < count; ++index) {
        deserializer.begin_element("behaviours", index);

        int type = 0;
        if (!deserializer.field("type", type)) {
            throw std::invalid_argument("behaviour type is missing");
        }

        add_behaviour(static_cast<BehaviourType>(type)).deserialize(deserializer);
        deserializer.end_element();
    }

    reset_behaviours();
}

const char* Entity::type_to_str(EntityType type) {
    switch (type) {
        case EntityType::BOX:
            return "box";
        case EntityType::PLATFORM:
            return "platform";
        case EntityType::SOLID:
            return "solid";
        case EntityType::SPIKE:
            return "spike";
        case EntityType::FINISH:
            return "finish";
        case EntityType::START:
            return "start";
        case EntityType::PARTICLES:
            return "particles";
        case EntityType::TEXT:
            return "text";
        case EntityType::STATIC_TEXTURE:
            return "static texture";
        case EntityType::TRIGGER:
            return "trigger";
        case EntityType::PLAYER:
            return "player";
        case EntityType::LEVEL:
            return "level";
        default:
            return "unknown";
    }
}
