#pragma once

#include "core/camera.hpp"

class EditorCameraControls {
public:
    void update(CameraView& camera, bool pointer_over_ui);
    void reset();

private:
    Vector2 m_drag_start_position = {};
    Vector2 m_drag_start_mouse_position = {};
    bool m_dragging = false;
};
