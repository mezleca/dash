#pragma once

#include "behaviour.hpp"
#include "game/objects/entities/player-type.hpp"

class DashLevel;

class SetPlayerType : public Behaviour {
public:
    explicit SetPlayerType(DashLevel& level);

    BehaviourType type() const override {
        return BehaviourType::PlayerType;
    }

    PlayerType player_type() const {
        return m_type;
    }

    void set_player_type(PlayerType type);
    void update(float frametime) override;
    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;

private:
    DashLevel& m_level;
    PlayerType m_type = PlayerType::BOX;
};
