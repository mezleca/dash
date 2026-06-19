#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../utils/json.hpp"

enum class ObjectType : int32_t { NONE = -1, BOX, PLATFORM, SPIKE, END, STATIC_TEXTURE };

struct RigidBody;

struct GameObject {
    explicit GameObject(ObjectType type);
    virtual ~GameObject();

    Texture2D texture = {};
    std::string texture_location;

    Vector2 velocity;
    std::unique_ptr<RigidBody> rb;
    Vector2 previous_position;
    Vector2 position;
    Vector2 dimensions;

    uint32_t id = 0;
    ObjectType type;
    bool visible;

    void load_texture(const char* location);

    virtual nlohmann::json serialize() const {
        return {{"type", static_cast<int>(type)},
                {"visible", visible},
                {"texture", texture_location},
                {"position", position},
                {"dimensions", dimensions}};
    }

    virtual void render() = 0;
};
