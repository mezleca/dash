#pragma once

#include <raylib.h>

constexpr float DEFAULT_CAMERA_ZOOM = 1.2f;
constexpr float DEFAULT_CAMERA_ROTATION = 0.0f;

class GameCamera {
public:
    const Camera2D& transform() const {
        return m_transform;
    }

    float zoom() const {
        return m_transform.zoom;
    }

    void set_zoom(float zoom) {
        m_transform.zoom = zoom;
    }

    float rotation() const {
        return m_transform.rotation;
    }

    void set_rotation(float rotation) {
        m_transform.rotation = rotation;
    }

    void reset() {
        m_transform.zoom = DEFAULT_CAMERA_ZOOM;
        m_transform.rotation = DEFAULT_CAMERA_ROTATION;
    }

    void set_target(Vector2 target, Vector2 smoothing = {1.0f, 1.0f});
    void center_viewport(Vector2 size);

private:
    Camera2D m_transform = {{}, {}, DEFAULT_CAMERA_ROTATION, DEFAULT_CAMERA_ZOOM};
};
