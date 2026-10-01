#pragma once

#include "../utils/json.hpp"

#include <filesystem>
#include <optional>

class GameObject;

class Component {
public:
    explicit Component(GameObject& object) : m_object(object) {}
    virtual ~Component() = default;

    GameObject& object() const {
        return m_object;
    }

    virtual void update(float) {}
    virtual void render(Rectangle, const Camera2D&) const {}

    virtual std::optional<nlohmann::json> serialize() const {
        return std::nullopt;
    }

    virtual void deserialize(const nlohmann::json&, const std::filesystem::path&) {}

private:
    GameObject& m_object;
};
