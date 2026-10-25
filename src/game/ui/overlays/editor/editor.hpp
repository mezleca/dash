#pragma once

#include <imgui-ui/layout/layer-container.hpp>
#include <functional>

namespace ui {
    class Surface;
} // namespace ui

class Dash;
class EditorPanel;
class BehaviourDialog;

class EditorOverlay : public ui::LayerContainer {
public:
    EditorOverlay(Dash& game, ui::Surface& ui_surface, ui::InputCallback on_key_press, std::function<void()> on_exit);

    bool has_unsaved_changes() const;
    void reset();

    bool close_behaviour_dialog();
    bool behaviour_dialog_open() const;

    void process_pointer_event(ui::UiEvent& event, bool pointer_over_ui);
    void update_input(bool active);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    bool input_available() const;

    Dash& m_game;

    EditorPanel* m_panel = nullptr;
    BehaviourDialog* m_behaviour_dialog = nullptr;
};
