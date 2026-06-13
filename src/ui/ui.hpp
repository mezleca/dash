#pragma once

#include <raylib.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <string_view>

enum class UIMode : int { NONE = 0, MENU = 1 << 0, OPTIONS = 1 << 1, PLAYFIELD = 1 << 2 };

enum UI_FONTS { BALOO = 0, FONT_COUNT };
enum UI_FONT_VAR { FONT_SMALL = 0, FONT_MEDIUM, FONT_LARGE, FONT_VAR_COUNT };

struct UIFont {
  private:
    ImFont* m_fonts[FONT_VAR_COUNT];

  public:
    ImFont*& operator[](UI_FONT_VAR var) {
        return m_fonts[var];
    }

    [[nodiscard]]
    ImFont* operator[](UI_FONT_VAR var) const {
        return m_fonts[var];
    }
};

struct UI {
  public:
    void initialize();

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

    // components
    bool render_level_button(std::string_view text, bool selected);
    bool render_button(std::string_view text, ImVec2 padding, ImVec2 size = {64.0f, 64.0f});

  private:
    ImGuiIO* m_io;
    Texture2D m_logo_texture;
    UIFont m_fonts[FONT_COUNT];
} inline g_ui;
