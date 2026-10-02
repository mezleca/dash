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

class GameUI : public ui::LayerContainer {
public:
    GameUI();

    void show_screen(GameScreen screen);
    void hide_screen(GameScreen screen);
    void hide_all_screens();

    GameScreen focused() const;

protected:
    void event(ui::UiEvent& event) override;

private:
    void build_menu();
    void build_option_panels();
    void build_gameplay_panels();

    void focus_current();

    void play_level(DashLevel& level);

    std::unordered_map<GameScreen, ui::Container*> m_screens;
    std::vector<GameScreen> m_open;
};
