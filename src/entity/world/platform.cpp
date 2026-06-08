#include "platform.hpp"
#include "../../physics/rigidbody.hpp"

#include <raylib.h>

Platform::Platform(float width, float height) : GameObject(ObjectType::PLAFORM) {
    rb->is_static = true;

    dimensions.x = width;
    dimensions.y = height;
}

void Platform::render() {
    if (!visible) return;
    DrawRectangleV(position, dimensions, {0, 120, 255, 255});
}
