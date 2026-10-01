#pragma once

#include "../entity.hpp"
#include "../../game/sprite.hpp"

class BackgroundSprite : public Sprite {
public:
    explicit BackgroundSprite(GameObject& object, bool fill_viewport = false);

    void render(Rectangle bounds, const Camera2D& camera) const override;

    std::optional<nlohmann::json> serialize() const override;
    void deserialize(const nlohmann::json& data, const std::filesystem::path& directory) override;

private:
    bool m_fill_viewport = false;
};

class StaticTexture : public Entity {
public:
    explicit StaticTexture(World& world, bool fill_viewport = false);
};
