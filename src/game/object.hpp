#pragma once

#include "component.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

enum class ObjectType : int32_t {
    NONE = -1,
    BOX,
    PLATFORM,
    SPIKE,
    END,
    STATIC_TEXTURE,
    TRIGGER
};

class GameObject {
public:
    explicit GameObject(ObjectType type);
    virtual ~GameObject() = default;

    template <class T, class... Args>
    T& add_component(Args&&... args) {
        auto component = std::make_unique<T>(*this, std::forward<Args>(args)...);
        T& result = *component;
        m_components.push_back(std::move(component));
        return result;
    }

    template <class T>
    T* get_component() const {
        for (const auto& component : m_components) {
            auto* result = dynamic_cast<T*>(component.get());

            if (result != nullptr) {
                return result;
            }
        }

        return nullptr;
    }

    ObjectType type() const {
        return m_type;
    }

    void update(float frametime);
    void render(Rectangle bounds, const Camera2D& camera) const;

    virtual nlohmann::json serialize() const;
    virtual void deserialize(const nlohmann::json& data, const std::filesystem::path& directory);

public:
    int z_index = 0;
    bool visible = true;

private:
    std::vector<std::unique_ptr<Component>> m_components;
    ObjectType m_type;
};
