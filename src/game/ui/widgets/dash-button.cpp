#include "dash-button.hpp"
#include <utility>

using namespace ui;

DashButton::DashButton(std::string name) : ButtonWidget(std::move(name)) {
    set_text_overflow(TextOverflow::Ellipsis);
}

void DashButton::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);

    configure_all_styles([](Style& style) {
        style.font_size(24);
        style.overflow(Overflow::Hidden);
    });
}
