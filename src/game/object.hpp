#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "../utils/json.hpp"

enum class ObjectType : int32_t {
    NONE = -1,
    BOX,
    PLATFORM,
    SPIKE,
    END,
    STATIC_TEXTURE
};

class GameObject {
public:
    explicit GameObject(ObjectType type);
    virtual ~GameObject();

    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    std::string texture_location;
    Texture2D texture = {};
    ObjectType type;
    int z_index = 0;
    bool visible = true;

    virtual void load_texture(const char* location);

    virtual nlohmann::json serialize() const;
    virtual void deserialize(const nlohmann::json& data, const std::filesystem::path& directory);

    virtual void render() = 0;
};
