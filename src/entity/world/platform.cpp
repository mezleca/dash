#include "platform.hpp"

#include <raylib.h>

Platform::Platform(World& world) : Entity(world, ObjectType::PLATFORM) {}

void Platform::render() {
    if (!visible) return;
    DrawRectangleRec(get_bounding_box(), {0, 120, 255, 255});
}
