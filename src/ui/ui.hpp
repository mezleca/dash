#pragma once

enum class UIMode : int { NONE = 0, MENU = 1 << 0, OPTIONS = 1 << 1, PLAYFIELD = 1 << 2 };

struct UI {
  public:
    UIMode mode;

    void render_main_menu();
    void render_debug_ui();
    void render_playfield_ui();

  private:
    bool m_is_pause_opened = false;
} inline g_ui;
