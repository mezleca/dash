#pragma once

#include "core/serialization/serialization.hpp"

#include <raylib.h>

#include <filesystem>

class GameObject;

enum class ComponentId : int {
    None = 0,
    RigidBody = 1,
    Sprite = 2,
    BackgroundSprite = 3
};

class Component {
public:
    explicit Component(GameObject& object, ComponentId id) : m_object(object), m_id(id) {}
    virtual ~Component() = default;

    GameObject& object() const {
        return m_object;
    }

    virtual void update(float) {}
    virtual void render(Rectangle, const Camera2D&) const {}

    ComponentId id() const {
        return m_id;
    }

    virtual void serialize(Serializer&) const {}
    virtual void deserialize(Deserializer&, const std::filesystem::path&) {}

private:
    GameObject& m_object;
    ComponentId m_id;
};
