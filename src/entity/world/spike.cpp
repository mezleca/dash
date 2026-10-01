#include "spike.hpp"

#include <raylib.h>
#include <stdexcept>

Spike::Spike(World& world, int amount) : Entity(world, ObjectType::SPIKE), m_amount(amount) {
    set_shape(make_box_shape(static_cast<float>(amount) * 64.0f, 64.0f));
}

void Spike::render() {
    if (!visible) return;

    const Vector2 position = get_position();
    const Vector2 dimensions = get_dimensions();
    const float width = dimensions.x / static_cast<float>(m_amount);

    for (int i = 0; i < m_amount; i++) {
        const float spacing = static_cast<float>(i) * width;

        DrawTriangle(
            {position.x + spacing + width / 2, position.y}, {position.x + spacing, position.y + dimensions.y},
            {position.x + spacing + width, position.y + dimensions.y}, {255, 0, 0, 255}
        );
    }
}

nlohmann::json Spike::serialize() const {
    auto data = Entity::serialize();
    data["spike_ammount"] = m_amount;
    return data;
}

void Spike::deserialize(const nlohmann::json& data, const std::filesystem::path& directory) {
    m_amount = data.value("spike_ammount", m_amount);
    if (m_amount <= 0) {
        throw std::invalid_argument("spike amount must be positive");
    }

    Entity::deserialize(data, directory);
}
