#pragma once

#include <imgui-ui/layout/layer-container.hpp>

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
class MenuOptionLayer;

class GameUI : public ui::LayerContainer {
public:
    GameUI();

    /// opens or focuses a panel without closing other panels. menu and gameplay clear the stack.
    void show(GameScreen screen);

    /// reports whether a panel is stacked, the menu is visible, or gameplay is the base screen.
    bool is_open(GameScreen screen) const;

    /// returns the front overlay, or the base screen when no overlay is open.
    GameScreen focused() const;

protected:
    void event(ui::UiEvent& event) override;

private:
    void build_menu();
    void build_option_panels();
    void build_gameplay_panels();
    void build_loading();

    MenuOptionLayer* option(GameScreen screen) const;

    void bring_to_front(ui::Node& node);
    void focus_current();
    void close_panel();
    void show_base(GameScreen screen);
    void show_loading();
    void hide_loading();

    void play_level(DashLevel& level);
    void load_level(DashLevel& level);

    ui::Container* m_menu = nullptr;
    MenuOptionLayer* m_levels = nullptr;
    MenuOptionLayer* m_editor = nullptr;
    MenuOptionLayer* m_settings = nullptr;
    MenuOptionLayer* m_exit = nullptr;
    MenuOptionLayer* m_pause = nullptr;
    MenuOptionLayer* m_death = nullptr;
    ui::LayerContainer* m_loading = nullptr;
    std::vector<GameScreen> m_open;
    GameScreen m_base = GameScreen::Menu;
};
