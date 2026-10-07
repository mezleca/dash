#pragma once

#include <imgui-ui/layout/layer-container.hpp>

namespace ui {
    class ImageWidget;
    class TextWidget;
} // namespace ui

class DebugInfo : public ui::LayerContainer {
public:
    DebugInfo();

protected:
    void on_update(float frametime) override;

private:
    ui::TextWidget* m_camera_position = nullptr;
    ui::TextWidget* m_object_count = nullptr;
    ui::TextWidget* m_camera_zoom = nullptr;
    ui::TextWidget* m_camera_rotation = nullptr;
    ui::TextWidget* m_frame_stats = nullptr;
};

class EditorPanel : public ui::LayerContainer {
public:
    EditorPanel();

private:
    void set_compact(bool compact);

    ui::ImageWidget* m_edit_icon = nullptr;
    ui::Container* m_content = nullptr;
};

class EditorLayer : public ui::LayerContainer {
public:
    EditorLayer();

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
