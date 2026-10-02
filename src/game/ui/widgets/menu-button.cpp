#include "menu-button.hpp"

#include <imgui-ui/surface.hpp>

#include <utility>

using namespace ui;

MenuButton::MenuButton(std::string text) : ButtonWidget(std::move(text)) {}

void MenuButton::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);

    set_font(surface().get_primary_font(28));
    TransitionSpec transition = {0.2F, easing::out_cubic};

    configure_all_styles([&](Style& style) {
        style.background_color(rgba(0, 0, 0, 0), transition);
        style.border(BORDER_BOTTOM);
        style.border_radius(0);
        style.border_color(rgb(200, 200, 200), transition);
        style.padding({12.0F, 2.0F});
    });

    style(StyleType::HOVER).border_color(surface().theme().accent_color, transition);
}
