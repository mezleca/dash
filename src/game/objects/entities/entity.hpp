#pragma once

#include "core/object.hpp"
#include "game/behaviours/behaviour.hpp"

#include <cstdint>
#include <memory>
#include <vector>

class DashLevel;

enum class EntityType : int32_t {
    NONE = -1,
    BOX,
    PLATFORM,
    SPIKE,
    STATIC_TEXTURE = 4,
    TRIGGER,
    PLAYER,
    LEVEL,
    FINISH,
    SOLID = 9,
    START,
    PARTICLES,
    TEXT,
};

class Entity : public GameObject {
public:
    explicit Entity(EntityType type);

    int id() const {
        return m_id;
    }

    void assign_new_id();

    DashLevel* level() const {
        return m_level;
    }

    void set_level(DashLevel& level) {
        m_level = &level;
    }

    EntityType type() const {
        return m_type;
    }

    const char* type_to_str() const {
        return type_to_str(m_type);
    }

    static const char* type_to_str(EntityType type);

    Behaviour& add_behaviour(BehaviourType type);
    void remove_behaviour(Behaviour& behaviour);

    const std::vector<std::unique_ptr<Behaviour>>& behaviours() const {
        return m_behaviours;
    }

    void update_behaviours(float frametime);
    void update(float frametime) override;
    virtual void reset_behaviours();

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

protected:
    void set_behaviours_active(bool active) {
        m_behaviours_active = active;
    }

private:
    EntityType m_type;
    int m_id = 0;
    DashLevel* m_level = nullptr;
    std::vector<std::unique_ptr<Behaviour>> m_behaviours;
    bool m_behaviours_active = true;
};
