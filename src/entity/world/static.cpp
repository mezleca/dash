#include "static.hpp"

BackgroundSprite::BackgroundSprite(GameObject& object, bool fill_viewport) : Sprite(object), m_fill_viewport(fill_viewport) {}

void BackgroundSprite::render(Rectangle bounds, const Camera2D& camera) const {
    Vector2 center = camera.target;

    if (m_fill_viewport) {
        bounds.width = static_cast<float>(GetScreenWidth());
        bounds.height = static_cast<float>(GetScreenHeight());
    } else {
        const Vector2 position = static_cast<const Entity&>(object()).get_position();
        center.x += position.x;
        center.y += position.y;
    }

    render_centered({center.x, center.y, bounds.width, bounds.height});
}

std::optional<nlohmann::json> BackgroundSprite::serialize() const {
    auto data = Sprite::serialize();
    (*data)["component"] = "background_sprite";
    if (m_fill_viewport) (*data)["fill_viewport"] = true;
    return data;
}

void BackgroundSprite::deserialize(const nlohmann::json& data, const std::filesystem::path& directory) {
    Sprite::deserialize(data, directory);
    m_fill_viewport = data.value("fill_viewport", false);
}

StaticTexture::StaticTexture(World& world, bool fill_viewport) : Entity(world, ObjectType::STATIC_TEXTURE) {
    add_component<BackgroundSprite>(fill_viewport);
}
