#pragma once

#include <raylib.h>
#include <cstdint>

enum class ObjectType : int32_t { NONE = -1, BOX, PLATFORM, SPIKE };

struct RigidBody;

struct GameObject {
    explicit GameObject(ObjectType type);
    ~GameObject();

    Texture2D texture;

    Vector2 velocity;
    RigidBody* rb;
    Vector2 previous_position;
    Vector2 position;
    Vector2 dimensions;

    uint32_t id = 0;
    ObjectType type;
    bool visible;

    void load_texture(const char* location);

    virtual void render() = 0;
};
