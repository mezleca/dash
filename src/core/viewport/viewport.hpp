#pragma once

#include "../camera.hpp"
#include <raylib.h>
#include <cstdint>

using ViewportId = uint32_t;
using ViewportMask = uint32_t;

static constexpr ViewportMask ALL_VIEWPORTS = 0xFFFFFFFFu;

class Viewport {
public:
    explicit Viewport(ViewportId id);
    ~Viewport();

    void begin();
    void end();
    void draw() const;

    Texture target() const {
        return m_target.texture;
    }

    CameraView& camera() {
        return m_camera;
    }

    const CameraView& camera() const {
        return m_camera;
    }

    ViewportMask mask() const {
        return 1u << m_id;
    }

    void set_position(Vector2 position);
    void set_size(Vector2 size);
    void set_background_color(Color color);

    Vector2 position() const {
        return m_position;
    }

    Vector2 size() const {
        return m_size;
    }

    Vector2 mouse_position() const;
    bool contains_mouse() const;

private:
    RenderTexture2D m_target = {};
    CameraView m_camera;
    Vector2 m_position = {0.0f, 0.0f};
    Vector2 m_size = {0.0f, 0.0f};
    ViewportId m_id = 0;
    Color m_background_color = BLACK;
};
