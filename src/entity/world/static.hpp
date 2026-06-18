#pragma once

#include "../../game/object.hpp"

struct StaticTexture : public GameObject {
    explicit StaticTexture(std::string_view location, bool fill_viewport);

    bool m_fill_viewport = false;

    void render() override;

    nlohmann::json serialize() const override {
        auto j = GameObject::serialize();
        j["fill_viewport"] = m_fill_viewport;
        return j;
    }
};
