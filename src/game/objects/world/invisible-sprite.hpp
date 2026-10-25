#pragma once

#include "core/components/sprite.hpp"

class DashLevel;

class InvisibleSprite : public Sprite {
public:
    InvisibleSprite(GameObject& object, const DashLevel& level, std::string label, Color tint = SKYBLUE);

    void select(bool selected);
    const std::string& label() const {
        return m_label;
    }
    void render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

private:
    const DashLevel& m_level;
    std::string m_label;
    bool m_selected = false;
};
