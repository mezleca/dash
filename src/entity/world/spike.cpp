#include "spike.hpp"
#include "../../physics/rigidbody.hpp"

#include <raylib.h>

Spike::Spike(float _ammount) {
    name = "spike";
    rb->is_static = true;

    ammount = _ammount;

    dimensions.x = ammount * 12.0f;
    dimensions.y = 12;
}

void Spike::render() {
    float width = dimensions.x / ammount;

    // Debug
    // DrawRectangleV(position, dimensions, RED);

    for (int i = 0; i < ammount; i++) {
        float spacing = i == 0 ? 0 : i * 12.0f;

        DrawTriangle(
            {position.x + spacing + width / 2, position.y},
            {position.x + spacing, position.y + dimensions.y},
            {position.x + spacing + width, position.y + dimensions.y},
            {255, 0, 0, 255}
        );
    }
}
