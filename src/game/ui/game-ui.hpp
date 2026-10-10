#pragma once

#include <imgui-ui/layout/layer-container.hpp>

#include <unordered_map>
#include <vector>

enum class GameScreen {
    Menu,
    Gameplay,
    Levels,
    Editor,
    Settings,
    Pause,
    Death,
    Loading,
    Exit,
};

class DashLevel;
class Dash;

class GameUI : public ui::LayerContainer {
public:
    explicit GameUI(Dash& game);

    void show_screen(GameScreen screen);
    void show_levels(bool editor = false);
    void hide_screen(GameScreen screen);
    void hide_all_screens();
    void pause_level();
    void resume_level();
    void return_to_menu();

    GameScreen focused() const;
    bool pointer_over_ui() const;

protected:
    void event(ui::UiEvent& event) override;

private:
    void focus_current();
    void play_level(DashLevel& level, bool editor = false);
    void finish_level_loading(bool loaded);

    Dash& m_game;
    std::unordered_map<GameScreen, ui::Container*> m_screens;
    std::vector<GameScreen> m_open;
    bool m_edit_selected_level = false;
};
