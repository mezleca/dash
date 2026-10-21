#pragma once

#include <memory>

namespace ui {
    class Runtime;
    class Surface;
} // namespace ui

enum class GameScreen {
    Menu,
    Gameplay,
    Levels,
    Editor,
    EditorExit,
    Settings,
    Death,
    Finished,
    Loading,
    Exit,
};

class Dash;
class GameScreens;

class GameUI {
public:
    explicit GameUI(Dash& game);
    ~GameUI();

    bool process_events();
    void update_editor();
    void draw(float frametime);
    void show_screen(GameScreen screen);

private:
    Dash& m_game;

    std::unique_ptr<ui::Runtime> m_runtime;
    std::unique_ptr<ui::Surface> m_surface;
    GameScreens* m_screens = nullptr;
};
