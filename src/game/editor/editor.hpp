#pragma once

#include "core/camera.hpp"
#include "game/level/level.hpp"

#include <cstddef>

class LevelEditor {
public:
    void update(CameraView& camera, DashLevel& level, bool pointer_over_ui, bool debugger_open);
    void pointer_down(CameraView& camera, DashLevel& level, Vector2 screen_position);
    void reset();
    bool clear_selection();

    Entity* selected_object(const DashLevel& level) const;

    std::size_t selection_version() const {
        return m_selection_version;
    }

private:
    Entity* object_at(const DashLevel& level, Vector2 world_position) const;
    void update_camera(CameraView& camera);
    void select(Entity* entity);

    DashLevel* m_level = nullptr;
    Entity* m_selected_entity = nullptr;
    Vector2 m_drag_start_position = {};
    Vector2 m_drag_start_mouse_position = {};
    std::size_t m_selection_version = 0;
    bool m_dragging = false;
};
