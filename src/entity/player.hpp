#pragma once

#include "../game/object.hpp"

struct Player : public GameObject {
    explicit Player();
    ~Player();

    float m_rotation = 0.0f;

    bool m_ignore_collision = false;
    bool m_should_lock_in_horizontally = false;

    void movement();
    void render() override;

    nlohmann::json serialize() const override {
        return GameObject::serialize();
    }
};
