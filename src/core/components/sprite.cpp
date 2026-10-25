#include "sprite.hpp"
#include "rigid-body.hpp"
#include "core/object.hpp"

#include <algorithm>
#include <cmath>
#include <rlgl.h>
#include <iostream>

Sprite::Sprite(GameObject& object, SpriteType sprite_type, Color color)
    : Component(object, ComponentId::Sprite), tint(color), type(sprite_type) {}

const char* Sprite::type_to_str(SpriteType type) {
    switch (type) {
        case SpriteType::TRIANGLE:
            return "triangle";
        case SpriteType::CIRCLE:
            return "circle";
        default:
            return "square";
    }
}

SpriteType Sprite::type_from_str(std::string_view type) {
    if (type == "triangle") return SpriteType::TRIANGLE;
    if (type == "circle") return SpriteType::CIRCLE;
    return SpriteType::SQUARE;
}

Sprite::~Sprite() {
    if (m_texture.id) {
        UnloadTexture(m_texture);
    }
}

bool Sprite::load_texture(const char* location) {
    Texture2D texture = {};
    if (location[0] != '\0') texture = LoadTexture(location);
    if (location[0] != '\0' && !IsTextureValid(texture)) {
        std::cout << "[sprite] failed to load texture from " << location << "\n";
        return false;
    }
    if (m_texture.id) UnloadTexture(m_texture);
    m_texture = texture;
    m_texture_location = location;
    return true;
}

void Sprite::render(Rectangle bounds, const Camera2D&, Vector2) const {
    bounds.x += bounds.width / 2.0f;
    bounds.y += bounds.height / 2.0f;
    render_centered(bounds);
}

void Sprite::render_centered(Rectangle bounds) const {
    // scale normalized vertices around the center.
    rlPushMatrix();
    rlTranslatef(bounds.x, bounds.y, 0.0f);
    rlRotatef(flip_x ? -rotation : rotation, 0.0f, 0.0f, 1.0f);
    rlScalef(bounds.width, bounds.height, 1.0f);

    const bool textured = IsTextureValid(m_texture);
    Color color = tint;
    color.a = static_cast<unsigned char>(static_cast<float>(tint.a) * std::clamp(opacity, 0.0F, 1.0F));

    if (type == SpriteType::SQUARE && textured) {
        const float width = static_cast<float>(m_texture.width);
        const Rectangle source = {0.0f, 0.0f, flip_x ? -width : width, static_cast<float>(m_texture.height)};
        DrawTexturePro(m_texture, source, {-0.5f, -0.5f, 1.0f, 1.0f}, {}, 0.0f, color);
    } else if (type == SpriteType::SQUARE) {
        DrawRectanglePro({-0.5f, -0.5f, 1.0f, 1.0f}, {}, 0.0f, color);
    } else if (textured) {
        render_textured_shape(color);
    } else if (type == SpriteType::TRIANGLE) {
        DrawTriangle({0.0f, -0.5f}, {-0.5f, 0.5f}, {0.5f, 0.5f}, color);
    } else {
        DrawEllipseV({}, 0.5f, 0.5f, color);
    }

    rlPopMatrix();
}

void Sprite::render_textured_shape(Color color) const {
    rlSetTexture(m_texture.id);
    rlBegin(RL_TRIANGLES);
    rlColor4ub(color.r, color.g, color.b, color.a);

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
    opacity = 1.0F;

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

    if (!texture.empty()) {
        load_texture((directory / texture).c_str());
        m_texture_location = texture;
    } else {
        load_texture("");
    }

    if (auto* body = object().get_component<RigidBody>(); body != nullptr && body->get_body_type() != b2_dynamicBody) {
        body->set_rotation(rotation);
    }
}
