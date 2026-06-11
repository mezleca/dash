#pragma once

#include <raylib.h>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <string>

enum class ObjectType : int32_t { NONE = -1, BOX, PLATFORM, SPIKE, END };

struct RigidBody;

inline static void to_json(nlohmann::json& j, const Vector2& v) {
    j = {{"x", v.x}, {"y", v.y}};
}

inline static void from_json(const nlohmann::json& j, Vector2& v) {
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
}

struct GameObject {
    explicit GameObject(ObjectType type);
    virtual ~GameObject();

    Texture2D texture;
    std::string texture_location;

    Vector2 velocity;
    RigidBody* rb;
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
