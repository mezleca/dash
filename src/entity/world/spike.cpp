#include "../../game/game.hpp"
#include "spike.hpp"
#include "../../physics/rigidbody.hpp"

#include <raylib.h>

Spike::Spike(int ammount) : GameObject(ObjectType::SPIKE), m_ammount(ammount) {
    game.add_game_object(this);
    rb->is_static = true;

    dimensions.x = static_cast<float>(ammount) * 64.0f;
    dimensions.y = 64.0f;
}

void Spike::render() {
    if (!visible) return;

    float width = dimensions.x / static_cast<float>(m_ammount);

    // Debug
    // DrawRectangleV(position, dimensions, RED);

    for (int i = 0; i < m_ammount; i++) {
        float spacing = i == 0 ? 0.0f : static_cast<float>(i) * width;

        DrawTriangle({position.x + spacing + width / 2, position.y}, {position.x + spacing, position.y + dimensions.y},
                     {position.x + spacing + width, position.y + dimensions.y}, {255, 0, 0, 255});
    }
}
