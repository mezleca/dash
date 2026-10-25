#include "viewport.hpp"

#include <iostream>

Viewport::Viewport(ViewportId id) : m_id(id) {}

Viewport::~Viewport() {
    if (m_target.id != 0) {
        UnloadRenderTexture(m_target);
    }
}

void Viewport::set_position(Vector2 position) {
    m_position = position;
}

void Viewport::set_size(Vector2 size) {
    m_size = size;
}

void Viewport::set_background_color(Color color) {
    m_background_color = color;
}

Vector2 Viewport::mouse_position() const {
    const Vector2 mouse = GetMousePosition();
    return {mouse.x - m_position.x, mouse.y - m_position.y};
}

bool Viewport::contains_mouse() const {
    const Vector2 mouse = mouse_position();
    return mouse.x >= 0.0f && mouse.x <= m_size.x && mouse.y >= 0.0f && mouse.y <= m_size.y;
}

void Viewport::begin() {
    bool has_different_size =
        m_target.texture.width != static_cast<int>(m_size.x) || m_target.texture.height != static_cast<int>(m_size.y);

    if (has_different_size && m_size.x > 0.0f) {
        if (m_target.id != 0) {
            UnloadRenderTexture(m_target);
        }

        m_target = LoadRenderTexture(static_cast<int>(m_size.x), static_cast<int>(m_size.y));
    }

    if (m_target.id == 0) {
        std::cout << "[game]: skipping viewport (invalid id)\n";
        return;
    }

    m_camera.center_viewport(m_size);

    BeginTextureMode(m_target);
    ClearBackground(m_background_color);
}

void Viewport::end() {
    if (m_target.id == 0) {
        std::cout << "[game]: skipping viewport (invalid id)\n";
        return;
    }

    EndTextureMode();
}

void Viewport::draw() const {
    if (m_target.id == 0 || m_size.x <= 0.0f || m_size.y <= 0.0f) {
        return;
    }

    const Rectangle source = {
        0.0f, 0.0f, static_cast<float>(m_target.texture.width), -static_cast<float>(m_target.texture.height)
    };
    const Rectangle destination = {m_position.x, m_position.y, m_size.x, m_size.y};
    DrawTexturePro(m_target.texture, source, destination, {}, 0.0f, WHITE);
}
