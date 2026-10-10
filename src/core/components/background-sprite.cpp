#include "background-sprite.hpp"
#include "rigid-body.hpp"
#include "core/object.hpp"

BackgroundSprite::BackgroundSprite(GameObject& object, bool fill_viewport)
    : Sprite(object, ComponentId::BackgroundSprite, SpriteType::SQUARE, WHITE), m_fill_viewport(fill_viewport) {}

void BackgroundSprite::render(Rectangle bounds, const Camera2D& camera) const {
    Vector2 center = camera.target;

    if (m_fill_viewport) {
        bounds.width = static_cast<float>(GetScreenWidth());
        bounds.height = static_cast<float>(GetScreenHeight());
    } else {
        const Vector2 position = object().get_component<RigidBody>()->get_position();
        center.x += position.x;
        center.y += position.y;
    }

    render_centered({center.x, center.y, bounds.width, bounds.height});
}

void BackgroundSprite::serialize(Serializer& serializer) const {
    Sprite::serialize(serializer);
    if (m_fill_viewport) serializer.field("fill_viewport", true);
}

void BackgroundSprite::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    Sprite::deserialize(deserializer, directory);
    m_fill_viewport = false;
    deserializer.field("fill_viewport", m_fill_viewport);
}
