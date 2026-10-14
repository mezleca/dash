#pragma once

#include "core/object.hpp"

#include <cstdint>

enum class EntityType : int32_t {
    NONE = -1,
    BOX,
    PLATFORM,
    SPIKE,
    END,
    STATIC_TEXTURE,
    TRIGGER,
    PLAYER,
    LEVEL
};

class Entity : public GameObject {
public:
    explicit Entity(EntityType type) : m_type(type) {}

    EntityType type() const {
        return m_type;
    }

    const char* type_to_str() const {
        return type_to_str(m_type);
    }

    static const char* type_to_str(EntityType type);

private:
    EntityType m_type;
};
