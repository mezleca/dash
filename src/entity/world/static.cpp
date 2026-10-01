#include "static.hpp"
#include "../../game/game.hpp"

StaticTexture::StaticTexture(World& world, bool fill_viewport)
    : Entity(world, ObjectType::STATIC_TEXTURE), m_fill_viewport(fill_viewport) {}

void StaticTexture::render() {
    if (!visible) {
        return;
    }

    Vector2 target_pos = game.m_camera.target;

    float target_width = static_cast<float>(texture.width);
    float target_height = static_cast<float>(texture.height);

    Rectangle source = {0.0f, 0.0f, target_width, target_height};

    if (m_fill_viewport) {
        target_width = static_cast<float>(GetScreenWidth());
        target_height = static_cast<float>(GetScreenHeight());
    } else {
        const Vector2 dimensions = get_dimensions();
        const Vector2 position = get_position();

        target_width = dimensions.x;
        target_height = dimensions.y;

        // use position as offset
        target_pos.x += position.x;
        target_pos.y += position.y;
    }

    Rectangle dest = {target_pos.x, target_pos.y, target_width, target_height};
    Vector2 origin = {target_width / 2, target_height / 2};

    DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
}

nlohmann::json StaticTexture::serialize() const {
    auto data = Entity::serialize();
    data["fill_viewport"] = m_fill_viewport;
    return data;
}

void StaticTexture::deserialize(const nlohmann::json& data, const std::filesystem::path& directory) {
    m_fill_viewport = data.value("fill_viewport", m_fill_viewport);

    Entity::deserialize(data, directory);
}
