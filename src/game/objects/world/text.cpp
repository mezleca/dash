#include "text.hpp"

#include "core/components/rigid-body.hpp"
#include "core/physics/shape.hpp"

#include <algorithm>
#include <rlgl.h>

TextSprite::TextSprite(GameObject& object) : Sprite(object) {
    m_font = LoadFontEx("resources/fonts/Baloo-Regular.ttf", 128, nullptr, 0);
    if (m_font.texture.id != 0) SetTextureFilter(m_font.texture, TEXTURE_FILTER_BILINEAR);
}

TextSprite::~TextSprite() {
    if (m_font.texture.id != 0) UnloadFont(m_font);
}

void TextSprite::render(Rectangle bounds, const Camera2D&, Vector2) const {
    if (text.empty() || m_font.texture.id == 0) return;

    const Vector2 size = MeasureTextEx(m_font, text.c_str(), 128, 0);
    if (size.x <= 0 || size.y <= 0) return;

    Color color = tint;
    color.a = static_cast<unsigned char>(static_cast<float>(color.a) * std::clamp(opacity, 0.0F, 1.0F));

    rlPushMatrix();
    rlTranslatef(bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F, 0);
    rlRotatef(rotation, 0, 0, 1);
    rlScalef((flip_x ? -bounds.width : bounds.width) / size.x, bounds.height / size.y, 1);
    DrawTextEx(m_font, text.c_str(), {-size.x * 0.5F, -size.y * 0.5F}, 128, 0, color);
    rlPopMatrix();
}

void TextSprite::serialize(Serializer& serializer) const {
    Sprite::serialize(serializer);
    serializer.field("text", text);
}

void TextSprite::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    Sprite::deserialize(deserializer, directory);
    text = "text";
    deserializer.field("text", text);
}

Text::Text(World& world) : Entity(EntityType::TEXT) {
    auto& body = add_component<RigidBody>(world, b2_staticBody, true, false);
    body.set_shape(make_box_shape(256, 64));
    add_component<TextSprite>();
}
