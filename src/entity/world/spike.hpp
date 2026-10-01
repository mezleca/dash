#pragma once

#include "../entity.hpp"

class Spike : public Entity {
public:
    explicit Spike(World& world, int amount);

    int m_amount = 0;

    void render() override;

    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& data, const std::filesystem::path& directory) override;
};
