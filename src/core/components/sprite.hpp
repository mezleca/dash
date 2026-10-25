#pragma once

#include "component.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

enum class SpriteType : uint8_t {
    SQUARE,
    TRIANGLE,
    CIRCLE
};

class Sprite : public Component {
public:
    explicit Sprite(GameObject& object, SpriteType type = SpriteType::SQUARE, Color tint = WHITE);
    ~Sprite() override;

    static const char* type_to_str(SpriteType type);
    static SpriteType type_from_str(std::string_view type);

    bool load_texture(const char* location);
    const std::string& texture_location() const {
        return m_texture_location;
    }
    void render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const override;

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

protected:
    void render_centered(Rectangle bounds) const;

private:
    void render_textured_shape(Color color) const;

    std::string m_texture_location;
    Texture2D m_texture = {};

public:
    Color tint = WHITE;
    float rotation = 0.0f;
    SpriteType type = SpriteType::SQUARE;
    bool flip_x = false;
    float opacity = 1.0F; // runtime multiplier
};
