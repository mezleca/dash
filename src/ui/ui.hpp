#pragma once

enum class UIMode : int { NONE = 0, MENU = 1 << 0, OPTIONS = 1 << 1, PLAYFIELD = 1 << 2 };

struct UI {
  public:
    UIMode mode;

    // playfield state
    bool m_playfield_container_open = false;
    bool m_show_pause = false;
    bool m_show_dead = false;
    bool m_show_finished = false;

    void reset_playfield_state() {
        m_playfield_container_open = false;
        m_show_pause = false;
        m_show_dead = false;
        m_show_finished = false;
    }

    void render_main_menu();
    void render_debug_ui();
    void render_playfield_ui();
} inline g_ui;
