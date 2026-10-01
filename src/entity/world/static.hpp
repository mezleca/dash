#pragma once

#include "../entity.hpp"

class StaticTexture : public Entity {
public:
    explicit StaticTexture(World& world, bool fill_viewport);

    bool m_fill_viewport = false;

    void render() override;

    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& data, const std::filesystem::path& directory) override;
};
