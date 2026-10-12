#include "editor.hpp"

#include <raymath.h>

std::optional<std::size_t> LevelEditor::object_at(const DashLevel& level, Vector2 world_position) const {
    const auto& objects = level.objects();
    std::optional<std::size_t> selected;

    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto* body = objects[index].object->get_component<RigidBody>();
        if (body == nullptr || !CheckCollisionPointRec(world_position, body->get_bounding_box())) {
            continue;
        }

        if (!selected || objects[index].object->z_order() >= objects[*selected].object->z_order()) {
            selected = index;
        }
    }

    return selected;
}

void LevelEditor::select(std::optional<std::size_t> index) {
    if (m_selected_index == index) {
        return;
    }

    m_selected_index = index;
    ++m_selection_version;
}

void LevelEditor::update(CameraView& camera, DashLevel& level, bool pointer_over_ui, bool debugger_open) {
    if (m_level != &level) {
        reset();
        m_level = &level;
    }

    if (pointer_over_ui || debugger_open) {
        m_dragging = false;
        return;
    }

    update_camera(camera);
}

void LevelEditor::pointer_down(CameraView& camera, DashLevel& level, Vector2 screen_position) {
    if (m_level != &level) {
        reset();
        m_level = &level;
    }

    const auto picked = object_at(level, GetScreenToWorld2D(screen_position, camera.view()));
    if (picked) {
        select(picked);
    }

    m_dragging = !picked.has_value();
    if (m_dragging) {
        m_drag_start_position = camera.view().target;
        m_drag_start_mouse_position = screen_position;
    }
}

void LevelEditor::update_camera(CameraView& camera) {
    const float wheel_y = GetMouseWheelMoveV().y;
    if (wheel_y != 0.0F) {
        camera.view().zoom = Clamp(camera.view().zoom + wheel_y * 0.25F, 0.1F, 10.0F);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        m_dragging = false;
    }

    if (m_dragging) {
        const Vector2 mouse_delta = Vector2Subtract(m_drag_start_mouse_position, GetMousePosition());
        const Vector2 world_delta = Vector2Scale(mouse_delta, 1.0F / camera.view().zoom);
        camera.set_target(Vector2Add(m_drag_start_position, world_delta));
    }
}

void LevelEditor::reset() {
    m_dragging = false;
    m_level = nullptr;
    select(std::nullopt);
}

bool LevelEditor::clear_selection() {
    if (!m_selected_index) {
        return false;
    }

    select(std::nullopt);
    return true;
}

const LevelObject* LevelEditor::selected_object(const DashLevel& level) const {
    if (m_level != &level || !m_selected_index || *m_selected_index >= level.objects().size()) {
        return nullptr;
    }

    return &level.objects()[*m_selected_index];
}
