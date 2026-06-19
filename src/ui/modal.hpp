#pragma once

#include <string>
#include <string_view>

struct UI;

namespace ui_modal_id {
    static constexpr std::string_view DEBUG = "debug";
    static constexpr std::string_view EDITOR = "editor";
    static constexpr std::string_view LEVEL_SELECTOR = "level_selector";
    static constexpr std::string_view MENU = "menu";
    static constexpr std::string_view PLAYFIELD = "playfield";
} // namespace ui_modal_id

struct UIModal {
    explicit UIModal(UI* ui, std::string_view id) : m_ui(ui), m_id(id) {
    }
    virtual ~UIModal() = default;

    UI* m_ui;
    std::string m_id;

    virtual void on_remove() = 0;
    virtual void on_escape() = 0;
    virtual void render() = 0;
};
