#include "menu-button.hpp"

#include <imgui-ui/surface.hpp>

#include <utility>

using namespace ui;

MenuButton::MenuButton(std::string text) : ButtonWidget(std::move(text)) {}

void MenuButton::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(28));

    configure_all_styles([](Style& style) {
        style.background_color(rgb(36, 36, 36), {0.2F, easing::out_cubic});
        style.padding({16.0F, 12.0F});
    });
    style(StyleType::HOVER).background_color(rgb(36, 36, 46), {0.2F, easing::out_cubic});
}
