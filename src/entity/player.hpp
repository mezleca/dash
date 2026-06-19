#pragma once

#include "../game/object.hpp"

enum class PlayerType : int { NONE = -1, BOX, BIRD };

struct Player : public GameObject {
  public:
    explicit Player();
    ~Player();

    float m_rotation = 0.0f;

    bool m_ignore_collision = false;
    bool m_finished_level = false;
    bool m_dead = false;
    bool m_should_lock_in_horizontally = false;

    PlayerType m_player_type;

    void movement();
    void update_player_type(PlayerType player_type);

    void reset();
    void render() override;

    nlohmann::json serialize() const override {
        return GameObject::serialize();
    }

  private:
    bool m_should_flip_player = false;
};
