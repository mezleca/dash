#pragma once

enum class UIMode : int { NONE = 0, MENU = 1 << 0, OPTIONS = 1 << 1, PLAYFIELD = 1 << 2 };

struct UI {
    UIMode mode;

    void render_main_menu();
    void render_player_ui();
} inline g_ui;
