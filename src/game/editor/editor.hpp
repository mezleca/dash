#pragma once

#include "core/camera.hpp"
#include "game/level/level.hpp"

#include <cstddef>
#include <optional>

class LevelEditor {
public:
    void update(CameraView& camera, DashLevel& level, bool pointer_over_ui, bool debugger_open);
    void pointer_down(CameraView& camera, DashLevel& level, Vector2 screen_position);
    void reset();
    bool clear_selection();

    const LevelObject* selected_object(const DashLevel& level) const;

    std::size_t selection_version() const {
        return m_selection_version;
    }

private:
    std::optional<std::size_t> object_at(const DashLevel& level, Vector2 world_position) const;
    void update_camera(CameraView& camera);
    void select(std::optional<std::size_t> index);

    DashLevel* m_level = nullptr;
    std::optional<std::size_t> m_selected_index;
    Vector2 m_drag_start_position = {};
    Vector2 m_drag_start_mouse_position = {};
    std::size_t m_selection_version = 0;
    bool m_dragging = false;
};
