#include "object.hpp"

#include <iostream>

GameObject::GameObject(ObjectType object_type) : type(object_type) {}

GameObject::~GameObject() {
    if (texture.id) {
        UnloadTexture(texture);
    }
}

void GameObject::load_texture(const char* location) {
    if (texture.id) {
        UnloadTexture(texture);
    }

    texture = LoadTexture(location);

    if (!IsTextureValid(texture)) {
        std::cout << "[object] failed to load texture from " << location << "\n";
        texture = {};
        texture_location.clear();
        return;
    }

    texture_location = location;
}

nlohmann::json GameObject::serialize() const {
    return {{"type", type}, {"visible", visible}, {"texture", texture_location}, {"z_index", z_index}};
}

void GameObject::deserialize(const nlohmann::json& data, const std::filesystem::path& directory) {
    visible = data.value("visible", true);
    z_index = data.value("z_index", 0);
    const std::string location = data.value("texture", std::string());

    // load relative to the level directory, but keep the original path for saving.
    if (!location.empty()) {
        load_texture((directory / location).c_str());
        texture_location = location;
    }
}
