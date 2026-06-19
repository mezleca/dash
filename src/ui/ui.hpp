#pragma once

#include "modal.hpp"
#include "modals/debug.hpp"
#include "modals/level.hpp"
#include "modals/playfield.hpp"
#include "modals/menu.hpp"

#include <vector>
#include <imgui.h>
#include <memory>
#include <string_view>

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
    ~UI();

    void initialize();
    void shutdown();

    UIFont m_fonts[FONT_COUNT];
    ImVec2 m_container_region = {0.0f, 0.0f};

    std::unique_ptr<DebugModal> m_debug_modal = nullptr;
    std::unique_ptr<PlayfieldModal> m_playfield_modal = nullptr;
    std::unique_ptr<LevelSelectorModal> m_level_selector_modal = nullptr;
    std::unique_ptr<MenuModal> m_menu_modal = nullptr;

    [[nodiscard]]
    bool is_modal_focused(UIModal* modal) const;

    [[nodiscard]]
    bool has_modal(std::string_view id) const;

    [[nodiscard]]
    UIModal* focused_modal() const;

    [[nodiscard]]
    size_t modal_count() {
        return m_modals.size();
    }

    void show_modal(UIModal* modal, bool wipe = false);
    bool remove_modal(std::string_view id);
    bool remove_focused_modal();
    void clear_modals();
    void handle_escape();

    void render();

    bool render_level_button(std::string_view text, std::string_view id, ImVec2 size = {0, 0}, bool selected = false);
    void render_progress_bar(std::string_view id, float a, float b, ImVec2 pos, ImVec2 size = {128.0f, 16.0f});
    bool render_button(std::string_view text, ImVec2 padding, ImVec2 size = {64.0f, 64.0f});
    bool render_menu_button(std::string_view text, ImVec2 padding, ImVec2 size = {80.0f, 40.0f});

  private:
    ImGuiIO* m_io = nullptr;

    std::vector<UIModal*> m_modals;
};
