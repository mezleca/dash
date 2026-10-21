#pragma once

#include <raylib.h>

#include <optional>
#include <string>
#include <vector>

class DashLevel;
class Entity;
class Viewport;

enum class EditorMode {
    Move,
    Select,
};

enum class InteractionMode {
    None,
    Panning,
    Moving,
    Resizing,
    Selecting,
};

struct EditorObjectState {
    Entity* m_entity = nullptr;
    Vector2 m_position = {};
    Vector2 m_size = {};
    float m_rotation = 0.0F;
};

struct EditorInteraction {
    InteractionMode m_mode = InteractionMode::None;
    Vector2 m_start = {}; // relative to viewport
    Vector2 m_end = {}; // relative to viewport
    Camera2D m_camera = {};
    Vector2 m_resize_direction = {};
    Rectangle m_bounds = {};
    std::vector<EditorObjectState> m_objects;
};

class LevelEditor {
public:
    LevelEditor(Viewport& viewport, DashLevel& level);

    void update(bool pointer_over_ui, bool debugger_open);
    void pointer_down(Vector2 screen_position);
    void pointer_move(Vector2 screen_position, bool snap = false);
    void pointer_up();
    void reset();
    bool clear_selection();

    void mark_changed();
    void mark_saved();
    bool has_unsaved_changes() const;

    void set_mode(EditorMode mode);
    EditorMode mode() const {
        return m_mode;
    }

    Entity* selected_object() const;

    const std::vector<Entity*>& selected_objects() const {
        return m_selection;
    }

    std::optional<Rectangle> selection_rectangle() const;

    bool manipulating_object() const {
        return m_interaction.m_mode == InteractionMode::Moving || m_interaction.m_mode == InteractionMode::Resizing;
    }

    static constexpr float JOINT_RADIUS = 6.0F;

private:
    Entity* object_at(Vector2 world_position) const;
    void begin_interaction(InteractionMode mode, Vector2 screen_position);
    void resize_selected(Vector2 delta);
    Vector2 snap_delta(Vector2 delta) const;

    Viewport& m_viewport;
    DashLevel& m_level;
    // the game owns these objects, the editor is reset before the level unloads
    std::vector<Entity*> m_selection;
    EditorInteraction m_interaction;
    EditorMode m_mode = EditorMode::Select;
    std::string m_saved_state;
    mutable bool m_changes_pending = false;
    mutable bool m_unsaved_changes = false;
};
