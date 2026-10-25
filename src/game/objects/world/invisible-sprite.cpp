#include "invisible-sprite.hpp"

#include "game/level/level.hpp"

#include <utility>

InvisibleSprite::InvisibleSprite(GameObject& object, const DashLevel& level, std::string label, Color color)
    : Sprite(object, SpriteType::SQUARE, color), m_level(level), m_label(std::move(label)) {
    select(false);
}

void InvisibleSprite::select(bool selected) {
    m_selected = selected;
    opacity = (selected ? 200.0F : 32.0F) / 255.0F;
}

void InvisibleSprite::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    Sprite::deserialize(deserializer, directory);
    select(m_selected);
}

void InvisibleSprite::render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const {
    if (m_level.state() != LevelState::EDITING) {
        return;
    }

    Sprite::render(bounds, camera, viewport_size);
}
