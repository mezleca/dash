#pragma once

#include "components/component.hpp"
#include "viewport/viewport.hpp"

#include <memory>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class GameObject {
public:
    GameObject() = default;
    virtual ~GameObject() = default;
    GameObject(const GameObject&) = delete;

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

    const std::vector<std::unique_ptr<Component>>& components() const {
        return m_components;
    }

    ViewportMask render_mask() const {
        return m_render_mask;
    }

    void set_render_mask(ViewportMask mask) {
        m_render_mask = mask;
    }

    int z_order() const {
        return z_index;
    }

    void set_z_order(int value) {
        z_index = value;
    }

    bool is_visible() const {
        return visible;
    }

    void set_visible(bool value) {
        visible = value;
    }

    void remove_component(Component& component);

    virtual void update(float frametime);
    void render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const;

    virtual void serialize(Serializer& serializer) const;
    virtual void deserialize(Deserializer& deserializer, const std::filesystem::path& directory);

    virtual void on_contact(GameObject&, Vector2) {}
    virtual void on_sensor(GameObject&) {}

    std::string serialize_to_buffer(SerializationFormat format = SerializationFormat::Json) const;
    void deserialize_from_buffer(
        std::string_view buffer, const std::filesystem::path& directory = {},
        SerializationFormat format = SerializationFormat::Json
    );

    bool save_to_file(const std::filesystem::path& file, SerializationFormat format = SerializationFormat::Auto) const;
    void load_from_file(const std::filesystem::path& file, SerializationFormat format = SerializationFormat::Auto);

protected:
    int z_index = 0;
    bool visible = true;

private:
    friend class Game;

    ViewportMask m_render_mask = ALL_VIEWPORTS;
    bool m_removed = false;
    std::vector<std::unique_ptr<Component>> m_components;
};
