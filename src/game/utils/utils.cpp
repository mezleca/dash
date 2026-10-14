#include "utils.hpp"

#include <raylib.h>

bool utils::is_mouse_within_window() {
    Vector2 mousePos = GetMousePosition();
    return (
        mousePos.x >= 0 && mousePos.x <= static_cast<float>(GetScreenWidth()) && mousePos.y >= 0 &&
        mousePos.y <= static_cast<float>(GetScreenHeight())
    );
}
