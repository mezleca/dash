#pragma once

#include "component.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

enum class SpriteType : uint8_t {
    SQUARE,
    TRIANGLE,
    CIRCLE
};

class Sprite : public Component {
public:
    explicit Sprite(GameObject& object, SpriteType type = SpriteType::SQUARE, Color tint = WHITE);
    ~Sprite() override;
    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
    Sprite(Sprite&&) = delete;
    Sprite& operator=(Sprite&&) = delete;

    void load_texture(const char* location);

    void render(Rectangle bounds, const Camera2D& camera) const override;

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

protected:
    Sprite(GameObject& object, ComponentId id, SpriteType type, Color tint);
    void render_centered(Rectangle bounds) const;

private:
    void render_textured_shape() const;

    std::string m_texture_location;
    Texture2D m_texture = {};

public:
    Color tint = WHITE;
    float rotation = 0.0f;
    SpriteType type = SpriteType::SQUARE;
    bool flip_x = false;
};
