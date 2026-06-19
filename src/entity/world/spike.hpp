#pragma once

#include "../../game/object.hpp"

struct Spike : public GameObject {
    explicit Spike(int ammount);

    int m_ammount = 0;

    void render() override;

    nlohmann::json serialize() const override {
        auto j = GameObject::serialize();
        j["spike_ammount"] = m_ammount;
        return j;
    }
};
