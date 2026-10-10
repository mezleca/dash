#pragma once

#include "sprite.hpp"

class BackgroundSprite : public Sprite {
public:
    explicit BackgroundSprite(GameObject& object, bool fill_viewport = false);

    void render(Rectangle bounds, const Camera2D& camera) const override;

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

    bool fill_viewport() const {
        return m_fill_viewport;
    }

    void set_fill_viewport(bool value) {
        m_fill_viewport = value;
    }

private:
    bool m_fill_viewport = false;
};
