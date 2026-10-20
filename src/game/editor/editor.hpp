#pragma once

#include "core/viewport/viewport.hpp"
#include "game/level/level.hpp"

#include <cstddef>

enum class EditorMode {
    Move,
    Select,
};

class LevelEditor {
public:
    LevelEditor(Viewport& viewport, DashLevel& level);

    void update(bool pointer_over_ui, bool debugger_open);
    void pointer_down(Vector2 screen_position);
    void reset();
    bool clear_selection();

    void set_mode(EditorMode mode);
    EditorMode mode() const {
        return m_mode;
    }

    Entity* selected_object() const;

    std::size_t selection_version() const {
        return m_selection_version;
    }

private:
    Entity* object_at(Vector2 world_position) const;
    void update_camera();
    void select(Entity* entity);

    Viewport& m_viewport;
    DashLevel& m_level;
    Entity* m_selected_entity = nullptr;
    Vector2 m_drag_start_position = {};
    Vector2 m_drag_start_mouse_position = {};
    std::size_t m_selection_version = 0;
    EditorMode m_mode = EditorMode::Select;
    bool m_dragging = false;
};
