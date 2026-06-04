#include "platform.hpp"
#include "../../physics/rigidbody.hpp"

#include <raylib.h>

Platform::Platform(float width, float height) {
    name = "platform";
    rb->is_static = true;

    // TEMP:
    dimensions.x = width;
    dimensions.y = height;
}

void Platform::render() {
    DrawRectangleV(position, dimensions, {0, 120, 255, 255});
}
