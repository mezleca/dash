#include "sprite.hpp"

#include <cmath>
#include <rlgl.h>
#include <iostream>

Sprite::Sprite(GameObject& object, SpriteType sprite_type, Color color)
    : Sprite(object, ComponentId::Sprite, sprite_type, color) {}

Sprite::Sprite(GameObject& object, ComponentId id, SpriteType sprite_type, Color color)
    : Component(object, id), tint(color), type(sprite_type) {}

Sprite::~Sprite() {
    if (m_texture.id) {
        UnloadTexture(m_texture);
    }
}

void Sprite::load_texture(const char* location) {
    if (m_texture.id) {
        UnloadTexture(m_texture);
    }

    m_texture = {};
    m_texture_location.clear();

    if (location[0] == '\0') return;

    m_texture = LoadTexture(location);

    if (!IsTextureValid(m_texture)) {
        std::cout << "[sprite] failed to load texture from " << location << "\n";
        m_texture = {};
        m_texture_location.clear();
        return;
    }

    m_texture_location = location;
}

void Sprite::render(Rectangle bounds, const Camera2D&) const {
    bounds.x += bounds.width / 2.0f;
    bounds.y += bounds.height / 2.0f;
    render_centered(bounds);
}

void Sprite::render_centered(Rectangle bounds) const {
    // place each shape at its center before scaling normalized vertices to the bounds.
    rlPushMatrix();
    rlTranslatef(bounds.x, bounds.y, 0.0f);
    rlRotatef(flip_x ? -rotation : rotation, 0.0f, 0.0f, 1.0f);
    rlScalef(bounds.width, bounds.height, 1.0f);

    const bool textured = IsTextureValid(m_texture);

    if (type == SpriteType::SQUARE && textured) {
        const float width = static_cast<float>(m_texture.width);
        const Rectangle source = {0.0f, 0.0f, flip_x ? -width : width, static_cast<float>(m_texture.height)};
        DrawTexturePro(m_texture, source, {-0.5f, -0.5f, 1.0f, 1.0f}, {}, 0.0f, tint);
    } else if (type == SpriteType::SQUARE) {
        DrawRectanglePro({-0.5f, -0.5f, 1.0f, 1.0f}, {}, 0.0f, tint);
    } else if (textured) {
        render_textured_shape();
    } else if (type == SpriteType::TRIANGLE) {
        DrawTriangle({0.0f, -0.5f}, {-0.5f, 0.5f}, {0.5f, 0.5f}, tint);
    } else {
        DrawEllipseV({}, 0.5f, 0.5f, tint);
    }

    rlPopMatrix();
}

void Sprite::render_textured_shape() const {
    rlSetTexture(m_texture.id);
    rlBegin(RL_TRIANGLES);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);

    const auto vertex = [&](float x, float y) {
        rlTexCoord2f(flip_x ? 0.5f - x : x + 0.5f, y + 0.5f);
        rlVertex2f(x, y);
    };

    if (type == SpriteType::TRIANGLE) {
        vertex(0.0f, -0.5f);
        vertex(-0.5f, 0.5f);
        vertex(0.5f, 0.5f);
    } else {
        constexpr int segments = 32;

        for (int i = 0; i < segments; ++i) {
            const float a = static_cast<float>(i) * 2.0f * PI / segments;
            const float b = static_cast<float>(i + 1) * 2.0f * PI / segments;
            vertex(0.0f, 0.0f);
            vertex(std::cos(b) * 0.5f, std::sin(b) * 0.5f);
            vertex(std::cos(a) * 0.5f, std::sin(a) * 0.5f);
        }
    }

    rlEnd();
    rlSetTexture(0);
}

void Sprite::serialize(Serializer& serializer) const {
    if (type != SpriteType::SQUARE) serializer.field("type", static_cast<int>(type));
    if (!m_texture_location.empty()) serializer.field("texture", m_texture_location);
    if (rotation != 0.0f) serializer.field("rotation", rotation);
    if (flip_x) serializer.field("flip_x", true);
    if (ColorToInt(tint) != ColorToInt(WHITE)) serializer.field("tint", tint);
}

void Sprite::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    int sprite_type = static_cast<int>(SpriteType::SQUARE);
    std::string texture;

    rotation = 0.0f;
    flip_x = false;
    tint = WHITE;

    deserializer.field("type", sprite_type);
    deserializer.field("texture", texture);
    deserializer.field("rotation", rotation);
    deserializer.field("flip_x", flip_x);
    deserializer.field("tint", tint);

    type = static_cast<SpriteType>(sprite_type);

    if (type > SpriteType::CIRCLE) {
        std::cerr << "[sprite] warning: invalid sprite type\nusing square\n";
        type = SpriteType::SQUARE;
    }

    if (!std::isfinite(rotation)) {
        std::cerr << "[sprite] warning: nonfinite rotation\nusing 0 degrees\n";
        rotation = 0.0f;
    }

    if (!texture.empty()) {
        load_texture((directory / texture).c_str());
        m_texture_location = texture;
    } else {
        load_texture("");
    }
}
