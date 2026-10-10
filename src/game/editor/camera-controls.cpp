#include "camera-controls.hpp"

#include <raymath.h>

void EditorCameraControls::update(CameraView& camera, bool pointer_over_ui) {
    if (pointer_over_ui) {
        m_dragging = false;
        return;
    }

    const float wheel_y = GetMouseWheelMoveV().y;
    if (wheel_y != 0.0f) {
        camera.view().zoom = Clamp(camera.view().zoom + wheel_y * 0.25f, 0.1f, 10.0f);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        m_dragging = true;
        m_drag_start_position = camera.view().target;
        m_drag_start_mouse_position = GetMousePosition();
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        m_dragging = false;
    }

    if (!m_dragging) {
        return;
    }

    const Vector2 mouse_delta = Vector2Subtract(m_drag_start_mouse_position, GetMousePosition());
    const Vector2 world_delta = Vector2Scale(mouse_delta, 1.0f / camera.view().zoom);
    camera.set_target(Vector2Add(m_drag_start_position, world_delta));
}

void EditorCameraControls::reset() {
    m_dragging = false;
}
