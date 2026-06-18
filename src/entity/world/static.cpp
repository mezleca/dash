#include "static.hpp"
#include "../../physics/rigidbody.hpp"
#include "../../game/game.hpp"

StaticTexture::StaticTexture(std::string_view location, bool fill_viewport)
    : GameObject(ObjectType::STATIC_TEXTURE), m_fill_viewport(fill_viewport) {
    game.add_game_object(this);
    rb->is_static = true;

    load_texture(location.data());
}

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
        // if our static thing has dimensions different then our texture, use it
        if (dimensions.x != target_width || dimensions.y != target_height) {
            target_width = static_cast<float>(dimensions.x);
            target_height = static_cast<float>(dimensions.y);
        }

        // use position as offset
        target_pos.x += position.x;
        target_pos.y += position.y;
    }

    Rectangle dest = {target_pos.x, target_pos.y, target_width, target_height};
    Vector2 origin = {target_width / 2, target_height / 2};

    DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
}
