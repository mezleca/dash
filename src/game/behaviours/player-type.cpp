#include "player-type.hpp"

#include "game/level/level.hpp"
#include "game/objects/entities/player.hpp"

#include <stdexcept>

SetPlayerType::SetPlayerType(DashLevel& level) : m_level(level) {}

void SetPlayerType::set_player_type(PlayerType type) {
    if (type != PlayerType::BOX && type != PlayerType::BIRD) {
        throw std::invalid_argument("invalid player type");
    }
    m_type = type;
}

void SetPlayerType::update(float) {
    auto* player = m_level.player();
    if (player == nullptr) return;

    player->update_player_type(m_type);
    finished();
}

void SetPlayerType::serialize(Serializer& serializer) const {
    serializer.field("player_type", static_cast<int>(m_type));
}

void SetPlayerType::deserialize(Deserializer& deserializer) {
    int type = static_cast<int>(PlayerType::BOX);
    deserializer.field("player_type", type);
    set_player_type(static_cast<PlayerType>(type));
}
