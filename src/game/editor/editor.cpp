#include "editor.hpp"

#include "core/components/rigid-body.hpp"

#include <algorithm>
#include <raymath.h>

LevelEditor::LevelEditor(Viewport& viewport, DashLevel& level) : m_viewport(viewport), m_level(level) {}

Entity* LevelEditor::object_at(Vector2 world_position) const {
    const auto& objects = m_level.objects();
    Entity* selected = nullptr;

    for (auto* object : objects) {
        const auto* body = object->get_component<RigidBody>();
        if (body == nullptr || !CheckCollisionPointRec(world_position, body->get_world_bounding_box())) {
            continue;
        }

        if (selected == nullptr || object->z_order() >= selected->z_order()) {
            selected = object;
        }
    }

    return selected;
}

void LevelEditor::select(Entity* entity) {
    if (m_selected_entity == entity) {
        return;
    }

    m_selected_entity = entity;
    ++m_selection_version;
}

void LevelEditor::update(bool pointer_over_ui, bool debugger_open) {
    if (pointer_over_ui || debugger_open || !m_viewport.contains_mouse()) {
        m_dragging = false;
        return;
    }

    update_camera();
}

void LevelEditor::pointer_down(Vector2 screen_position) {
    if (m_mode == EditorMode::Move) {
        m_dragging = true;
        m_drag_start_position = m_viewport.camera().view().target;
        m_drag_start_mouse_position = screen_position;
        return;
    }

    const auto picked = object_at(GetScreenToWorld2D(screen_position, m_viewport.camera().view()));
    if (picked != nullptr) {
        select(picked);
    }

    m_dragging = picked == nullptr;
    if (m_dragging) {
        m_drag_start_position = m_viewport.camera().view().target;
        m_drag_start_mouse_position = screen_position;
    }
}

void LevelEditor::set_mode(EditorMode mode) {
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    if (mode == EditorMode::Move) {
        select(nullptr);
    }
}

void LevelEditor::update_camera() {
    auto& camera = m_viewport.camera();

    const float wheel_y = GetMouseWheelMoveV().y;
    if (wheel_y != 0.0F) {
        camera.view().zoom = Clamp(camera.view().zoom + wheel_y * 0.25F, 0.1F, 10.0F);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        m_dragging = false;
    }

    if (m_dragging) {
        const Vector2 mouse_delta = Vector2Subtract(m_drag_start_mouse_position, m_viewport.mouse_position());
        const Vector2 world_delta = Vector2Scale(mouse_delta, 1.0F / camera.view().zoom);
        camera.set_target(Vector2Add(m_drag_start_position, world_delta));
    }
}

void LevelEditor::reset() {
    m_dragging = false;
    select(nullptr);
}

bool LevelEditor::clear_selection() {
    if (m_selected_entity == nullptr) {
        return false;
    }

    select(nullptr);
    return true;
}

Entity* LevelEditor::selected_object() const {
    if (m_selected_entity == nullptr) {
        return nullptr;
    }

    const auto& objects = m_level.objects();
    return std::find(objects.begin(), objects.end(), m_selected_entity) != objects.end() ? m_selected_entity : nullptr;
}
