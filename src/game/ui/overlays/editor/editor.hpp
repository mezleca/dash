#pragma once

#include <imgui-ui/layout/layer-container.hpp>

#include <cstddef>
#include <string>

namespace ui {
    class Container;
    class ImageWidget;
    class TextWidget;
} // namespace ui

class Dash;
class DashLevel;
class EditorModeButton;
class EditorTabButton;
class LevelEditor;
enum class EditorMode;

class DebugPanel : public ui::LayerContainer {
public:
    explicit DebugPanel(Dash& game);

protected:
    void on_update(float frametime) override;

private:
    Dash& m_game;

    ui::TextWidget* m_camera_position = nullptr;
    ui::TextWidget* m_object_count = nullptr;
    ui::TextWidget* m_camera_zoom = nullptr;
    ui::TextWidget* m_camera_rotation = nullptr;
    ui::TextWidget* m_frame_stats = nullptr;
};

class EditorPanel : public ui::LayerContainer {
public:
    EditorPanel(Dash& game, ui::InputCallback on_key_press);

protected:
    void on_update(float frametime) override;

private:
    void key_press_event(ui::UiEvent& event) override;
    void set_mode(EditorMode mode);
    void build_library(ui::Container& body);

    Dash& m_game;
    ui::InputCallback m_on_key_press;
    ui::Container* m_game_space = nullptr;
    ui::Container* m_tab_body = nullptr;
    EditorTabButton* m_library_tab = nullptr;
    EditorModeButton* m_move_button = nullptr;
    EditorModeButton* m_select_button = nullptr;
    LevelEditor* m_editor = nullptr;
    bool m_initialized = false;
};

class EditorOverlay : public ui::LayerContainer {
public:
    EditorOverlay(Dash& game, ui::InputCallback on_key_press);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
