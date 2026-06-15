#pragma once

#include <string>
#include <string_view>

struct UI;

namespace ui_modal_id {
    static constexpr std::string_view DEBUG = "debug";
    static constexpr std::string_view DEATH = "death";
    static constexpr std::string_view FINISH = "finish";
    static constexpr std::string_view LEVEL_SELECTOR = "level_selector";
    static constexpr std::string_view MENU = "menu";
    static constexpr std::string_view PAUSE = "pause";
    static constexpr std::string_view PLAYER = "player";
} // namespace ui_modal_id

struct UIModal {
    explicit UIModal(UI* ui, std::string_view id, bool remove_on_escape = false)
        : m_ui(ui), m_id(id), m_remove_on_escape(remove_on_escape) {
    }

    UI* m_ui;
    std::string m_id;
    bool m_remove_on_escape = false;

    virtual void on_remove() {
    }
    virtual void render() = 0;
};
