#include "editor-tab-button.hpp"

#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

EditorTabButton::EditorTabButton(std::string name) : ButtonWidget(std::move(name)) {
    set_text_overflow(TextOverflow::Ellipsis);
}

void EditorTabButton::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);

    configure_all_styles([](Style& style) {
        style.font_size(25);
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(0.0F);
        style.padding({14.0F, 6.0F});
        style.background_color(rgba(0, 0, 0, 0));
    });

    update_style();
}

void EditorTabButton::set_selected(bool value) {
    if (m_selected == value) {
        return;
    }

    m_selected = value;
    update_style();
}

void EditorTabButton::update_style() {
    const Theme theme = surface().theme();
    const TransitionSpec transition(0.15F, easing::out_cubic);

    style(StyleType::DEFAULT).background_color(m_selected ? theme.background_secondary_color : rgba(0, 0, 0, 0), transition);
    style(StyleType::HOVER).background_color(theme.background_secondary_color, transition);
    style(StyleType::ACTIVE).background_color(theme.background_secondary_color, transition);
}
