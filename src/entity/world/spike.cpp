#include "spike.hpp"
#include "../../physics/rigidbody.hpp"

#include <raylib.h>

Spike::Spike(float _ammount) : GameObject(ObjectType::SPIKE) {
    rb->is_static = true;

    ammount = _ammount;

    dimensions.x = ammount * 64.0f;
    dimensions.y = 64.0f;
}

void Spike::render() {
    if (!visible) return;

    float width = dimensions.x / ammount;

    // Debug
    // DrawRectangleV(position, dimensions, RED);

    for (int i = 0; i < ammount; i++) {
        float spacing = i == 0 ? 0 : i * width;

        DrawTriangle({position.x + spacing + width / 2, position.y}, {position.x + spacing, position.y + dimensions.y},
                     {position.x + spacing + width, position.y + dimensions.y}, {255, 0, 0, 255});
    }
}
