#pragma once

#include "core/components/sprite.hpp"

enum class ImageSpace {
    World,
    Static,
};

class ImageSprite : public Sprite {
public:
    explicit ImageSprite(GameObject& object, ImageSpace space = ImageSpace::World, bool fill_viewport = false);

    void render(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const override;
    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

    ImageSpace space() const {
        return m_space;
    }

    // preserve visible geometry across spaces
    void set_space(ImageSpace space, const Camera2D& camera, Vector2 viewport_size);
    Camera2D render_camera(const Camera2D& camera) const;
    Rectangle render_bounds(Rectangle bounds, const Camera2D& camera, Vector2 viewport_size) const;

    // only static images can fill the viewport
    bool fill_viewport() const {
        return m_space == ImageSpace::Static && m_fill_viewport;
    }
    void set_fill_viewport(bool value) {
        m_fill_viewport = value;
    }

private:
    ImageSpace m_space = ImageSpace::World;
    bool m_fill_viewport = false;
};
