#pragma once

#include <raylib.h>

class CameraView {
public:
    explicit CameraView(float zoom = 1.0f) : m_view{{}, {}, 0.0f, zoom} {}

    Camera2D& view() {
        return m_view;
    }

    const Camera2D& view() const {
        return m_view;
    }

    void set_target(Vector2 target, Vector2 smoothing = {1.0f, 1.0f});
    void center_viewport(Vector2 size);

private:
    Camera2D m_view;
};
