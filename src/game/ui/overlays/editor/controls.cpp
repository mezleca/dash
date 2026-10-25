#include "controls.hpp"
#include "game/ui/theme.hpp"
#include <imgui-ui/surface.hpp>
#include <imgui-ui/style/styled-node.hpp>

using namespace ui;

EditorModeButton::EditorModeButton(Texture* icon, bool selected) : DashIconButton(icon), m_selected(selected) {
    set_size({px(38.0F), px(38.0F)});
}

void EditorModeButton::set_selected(bool value) {
    if (m_selected == value) {
        return;
    }

    m_selected = value;
    update_style(surface().theme());
}

void EditorModeButton::apply_theme_defaults(const Theme& theme) {
    DashIconButton::apply_theme_defaults(theme);
    update_style(theme);
}

void EditorModeButton::update_style(const Theme& theme) {
    const auto& transition = dash_ui::control_transition;

    configure_all_styles([&theme, &transition](Style& style) {
        style.padding({6.0F, 6.0F});
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(theme.controls.rounding);
        style.background_color(theme.controls.background_color, transition);
    });

    const Color muted = theme.text_secondary_color;
    const Color light = theme.text_color;

    style(StyleType::DEFAULT)
        .background_color(m_selected ? theme.accent_color : theme.controls.background_color, transition)
        .color(m_selected ? light : muted, transition);

    style(StyleType::HOVER)
        .background_color(m_selected ? theme.accent_hover_color : theme.controls.hover_color, transition)
        .border_color(theme.accent_color, transition)
        .color(light, transition);

    style(StyleType::ACTIVE).background_color(theme.controls.active_color, transition).color(light, transition);
}

EditorTabButton::EditorTabButton(std::string name, bool selected) : DashButton(std::move(name)), m_selected(selected) {}

void EditorTabButton::apply_theme_defaults(const Theme& theme) {
    DashButton::apply_theme_defaults(theme);

    configure_all_styles([](Style& style) {
        style.font_size(25);
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(0.0F);
        style.padding({8.0F, 4.0F});
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
    const Theme& theme = surface().theme();
    const auto& transition = dash_ui::control_transition;

    configure_all_styles([&](Style& style) { style.color(m_selected ? theme.accent_color : theme.text_color, transition); });
    style(StyleType::HOVER).color(theme.accent_hover_color, transition);
    style(StyleType::ACTIVE).color(theme.accent_color, transition);
}
