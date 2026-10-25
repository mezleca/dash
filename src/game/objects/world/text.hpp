#pragma once

#include "core/components/sprite.hpp"
#include "game/objects/entities/entity.hpp"

class World;

class TextSprite : public Sprite {
public:
    explicit TextSprite(GameObject& object);
    ~TextSprite() override;
    TextSprite(const TextSprite&) = delete;
    TextSprite& operator=(const TextSprite&) = delete;

    void render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const override;
    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

    std::string text = "text";

private:
    Font m_font = {};
};

class Text : public Entity {
public:
    explicit Text(World& world);
};
