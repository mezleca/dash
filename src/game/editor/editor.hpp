#pragma once

#include <raylib.h>

#include <optional>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

class DashLevel;
class Entity;
class Viewport;
struct LibraryEntry;
enum class EntityType : int32_t;

static constexpr int OBJECTS_BUFFER_LIMIT = 32;

enum class EditorMode {
    Move,
    Select,
    Add,
    PickTarget,
};

enum class InteractionMode {
    None,
    Panning,
    Moving,
    Resizing,
    Selecting,
};

enum class EditorEventType {
    PointerDown,
    PointerMove,
    PointerUp,
    Scroll,
    FinishInteraction,
    Cancel,
    Delete,
};

struct EditorEvent {
    EditorEventType m_type;
    Vector2 m_position = {}; // relative to viewport
    float m_scroll = 0.0F;
    bool m_snap = false;
    int m_button = MOUSE_BUTTON_LEFT;
};

struct EditorObjectState {
    Entity* m_entity = nullptr;
    Vector2 m_position = {};
    Vector2 m_size = {};
    float m_rotation = 0.0F;
    Camera2D m_camera = {};
};

struct EditorInteraction {
    InteractionMode m_mode = InteractionMode::None;
    int m_button = MOUSE_BUTTON_LEFT;
    Vector2 m_start = {}; // relative to viewport
    Vector2 m_end = {}; // relative to viewport
    Camera2D m_camera = {};
    Vector2 m_resize_direction = {};
    Rectangle m_bounds = {};
    std::vector<EditorObjectState> m_objects;
};

struct EditorObjectData {
    EntityType m_type;
    std::string m_state;
    bool operator==(const EditorObjectData&) const = default;
};

struct EditorSnapshot {
    std::vector<EditorObjectData> m_objects;
    std::uint32_t m_solid_background = 0;
    bool operator==(const EditorSnapshot&) const = default;
};

class LevelEditor {
public:
    LevelEditor(Viewport& viewport, DashLevel& level);
    ~LevelEditor();
    LevelEditor(const LevelEditor&) = delete;
    LevelEditor& operator=(const LevelEditor&) = delete;
    LevelEditor(LevelEditor&&) = delete;
    LevelEditor& operator=(LevelEditor&&) = delete;

    void handle_event(const EditorEvent& event);
    void begin_add(const LibraryEntry& entry);
    void begin_pick_target(std::function<void(Entity*)> picked);
    void reset();
    bool clear_selection();

    void mark_changed();
    void commit_changes();
    void undo();
    void copy_selection();
    void paste();
    void delete_selection();
    void mark_saved();
    bool has_unsaved_changes() const;

    void set_mode(EditorMode mode);
    EditorMode mode() const {
        return m_mode;
    }

    int pointer_button() const {
        return m_interaction.m_button;
    }

    Entity* selected_object() const;

    const std::vector<Entity*>& selected_objects() const {
        return m_selection;
    }

    const Entity* preview_object() const {
        return m_preview;
    }

    std::optional<Rectangle> selection_rectangle() const;
    std::array<Vector2, 4> selection_corners(const Entity& object) const;

    bool manipulating_object() const {
        return m_interaction.m_mode == InteractionMode::Moving || m_interaction.m_mode == InteractionMode::Resizing;
    }

    static constexpr float JOINT_RADIUS = 6.0F;

private:
    Entity* object_at(Vector2 screen_position) const;
    void pick_target(const EditorEvent& event);
    void place_object(const EditorEvent& event);
    void start_interaction(const EditorEvent& event);
    void begin_interaction(InteractionMode mode, Vector2 screen_position);
    void finish_interaction();
    void update_selection();
    void move_preview(Vector2 world_position);
    void cancel_add();
    void resize_selected(Vector2 delta);
    Vector2 snap_delta(Vector2 delta) const;
    EditorSnapshot snapshot() const;

    Viewport& m_viewport;
    DashLevel& m_level;

    std::vector<Entity*> m_selection;

    std::vector<EditorObjectData> m_copied_objects;
    std::vector<EditorSnapshot> m_history;

    EditorSnapshot m_current_state;
    EditorInteraction m_interaction;
    EditorMode m_mode = EditorMode::Move;
    EditorMode m_previous_mode = EditorMode::Move;
    Entity* m_preview = nullptr;
    float m_preview_opacity = 1.0F;
    bool m_preview_collidable = false;
    std::function<void(Entity*)> m_target_picked;

    std::string m_saved_state;
    bool m_edit_pending = false;
    mutable bool m_changes_pending = false;
    mutable bool m_unsaved_changes = false;
};
