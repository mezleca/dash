#pragma once

#include <imgui-ui/layout/layer-container.hpp>

#include <cstddef>

namespace ui {
    class ImageWidget;
    class TextWidget;
} // namespace ui

class Dash;
class EditorTabs;

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
    void set_compact(bool compact);

    Dash& m_game;
    ui::InputCallback m_on_key_press;
    ui::ImageWidget* m_edit_icon = nullptr;
    ui::Container* m_content = nullptr;
    EditorTabs* m_tabs = nullptr;
    std::size_t m_selection_version = 0;
};

class EditorOverlay : public ui::LayerContainer {
public:
    EditorOverlay(Dash& game, ui::InputCallback on_key_press);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
