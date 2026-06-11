#pragma once

#include "../../game/object.hpp"

struct Finish : public GameObject {
    explicit Finish();

    float m_radius = 0.0f;
    float m_lum = 0.0f;

    void render() override;

    nlohmann::json serialize() const override {
        return GameObject::serialize();
    }
};
