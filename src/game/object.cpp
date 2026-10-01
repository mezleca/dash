#include "object.hpp"
#include "sprite.hpp"

#include <iostream>

GameObject::GameObject(ObjectType object_type) : m_type(object_type) {}

void GameObject::update(float frametime) {
    // components added by a callback start updating on the next frame.
    const size_t count = m_components.size();
    for (size_t i = 0; i < count; ++i) {
        m_components[i]->update(frametime);
    }
}

void GameObject::render(Rectangle bounds, const Camera2D& camera) const {
    if (!visible) return;

    // components added during drawing enter the next render pass.
    const size_t count = m_components.size();
    for (size_t i = 0; i < count; ++i) {
        m_components[i]->render(bounds, camera);
    }
}

nlohmann::json GameObject::serialize() const {
    nlohmann::json data = {{"type", m_type}, {"components", nlohmann::json::array()}};

    if (!visible) data["visible"] = false;
    if (z_index != 0) data["z_index"] = z_index;

    // components without saved data stay out of the level file.
    for (const auto& component : m_components) {
        auto options = component->serialize();
        if (options.has_value()) {
            data["components"].push_back(std::move(*options));
        }
    }

    return data;
}

void GameObject::deserialize(const nlohmann::json& data, const std::filesystem::path& directory) {
    visible = data.value("visible", true);
    z_index = data.value("z_index", 0);

    const auto components = data.find("components");
    // no saved components means removing constructor components that save level data.
    if (components == data.end() || components->empty()) {
        std::erase_if(m_components, [](const auto& component) { return component->serialize().has_value(); });
        return;
    }

    size_t next = 0;

    for (const auto& options : *components) {
        // skip runtime-only components before matching the next saved one.
        while (next < m_components.size() && !m_components[next]->serialize().has_value()) {
            ++next;
        }

        if (next < m_components.size()) {
            auto& component = m_components[next];

            if (component->serialize()->at("component") != options.at("component")) {
                std::cerr << "[object] warning: component type does not match object\nskipping component\n";
                continue;
            }

            component->deserialize(options, directory);
            ++next;
        } else if (options.at("component") == "sprite") {
            add_component<Sprite>().deserialize(options, directory);
            next = m_components.size();
        } else {
            std::cerr << "[object] warning: unknown component type\nskipping component\n";
        }
    }
}
