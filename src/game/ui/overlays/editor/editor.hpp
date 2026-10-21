#pragma once

#include <imgui-ui/layout/layer-container.hpp>
#include <functional>

namespace ui {
    class Surface;
} // namespace ui

class Dash;
class EditorPanel;

class EditorOverlay : public ui::LayerContainer {
public:
    EditorOverlay(Dash& game, ui::Surface& ui_surface, ui::InputCallback on_key_press, std::function<void()> on_exit);
    bool has_unsaved_changes() const;
    void reset();

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    EditorPanel* m_panel = nullptr;
};
