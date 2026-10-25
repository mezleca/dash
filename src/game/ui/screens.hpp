#pragma once

#include "ui.hpp"
#include <imgui-ui/layout/layer-container.hpp>
#include <unordered_map>
#include <vector>

class Dash;
class DashLevel;

class GameScreens : public ui::LayerContainer {
public:
    GameScreens(Dash& game, ui::Surface& ui_surface);

    void show_screen(GameScreen screen);
    void process_input(ui::UiEvent& event);
    void update_editor();
    void event(ui::UiEvent& event) override;

private:
    GameScreen focused() const;
    bool pointer_over_ui(ImVec2 position) const;
    bool editor_active() const;

    void request_editor_exit();
    void show_levels(bool editor);
    void hide_screen(GameScreen screen);
    void hide_all_screens();
    void focus_current();

    void play_level(DashLevel& level, bool editor);
    void finish_level_loading(bool loaded);

    Dash& m_game;

    std::unordered_map<GameScreen, ui::Container*> m_screens;
    std::vector<GameScreen> m_open;
    bool m_edit_selected_level = false;
};
