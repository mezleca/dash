#include "controls.hpp"
#include <imgui-ui/surface.hpp>
#include <imgui-ui/style/styled-node.hpp>

using namespace ui;

EditorIconButton::EditorIconButton(Texture* icon) : ImageWidget(icon) {
    set_input_mode(InputMode::Target);
    set_size({px(30.0F), px(30.0F)});
    set_fit(ImageFit::Contain);
}

void EditorIconButton::apply_theme_defaults(const Theme& theme) {
    ImageWidget::apply_theme_defaults(theme);

    const TransitionSpec transition(0.15F, easing::out_cubic);
    configure_all_styles([&theme, &transition](Style& style) {
        style.padding({5.0F, 5.0F});
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(theme.controls.rounding);
        style.background_color(rgba(0, 0, 0, 0), transition);
        style.color(theme.text_secondary_color, transition);
    });

    style(StyleType::HOVER).background_color(rgba(149, 204, 221, 40), transition).color(theme.text_color, transition);
    style(StyleType::ACTIVE).background_color(rgba(66, 116, 217, 120), transition);
}

void EditorIconButton::event(UiEvent& event) {
    if (event.type == EventType::PointerDown && event.button == PointerButton::Left) {
        event.stop_propagation();
        return;
    }

    if (event.type != EventType::Click || event.button != PointerButton::Left) {
        return;
    }

    if (m_on_click) {
        m_on_click();
    }

    event.stop_propagation();
}

EditorModeButton::EditorModeButton(Texture* icon, bool selected) : EditorIconButton(icon), m_selected(selected) {
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
    ImageWidget::apply_theme_defaults(theme);
    update_style(theme);
}

void EditorModeButton::update_style(const Theme& theme) {
    const TransitionSpec transition(0.15F, easing::out_cubic);

    configure_all_styles([&theme, &transition](Style& style) {
        style.padding({6.0F, 6.0F});
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(theme.controls.rounding);
        style.background_color(theme.controls.background_color, transition);
    });

    const Color muted = rgb(150, 158, 175);
    const Color light = rgb(214, 222, 232);

    style(StyleType::DEFAULT)
        .background_color(m_selected ? theme.accent_color : theme.controls.background_color, transition)
        .color(m_selected ? light : muted, transition);

    style(StyleType::HOVER)
        .background_color(m_selected ? theme.accent_hover_color : theme.controls.hover_color, transition)
        .border_color(theme.accent_color, transition)
        .color(light, transition);

    style(StyleType::ACTIVE).background_color(theme.controls.active_color, transition).color(light, transition);
}

EditorTabButton::EditorTabButton(std::string name) : DashButton(std::move(name)) {}

void EditorTabButton::apply_theme_defaults(const Theme& theme) {
    DashButton::apply_theme_defaults(theme);

    configure_all_styles([theme](Style& style) {
        style.font_size(25);
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(4.0F);
        style.padding({14.0F, 6.0F});
        style.background_color(theme.background_secondary_color);
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
    const TransitionSpec transition(0.15F, easing::out_cubic);

    style(StyleType::DEFAULT).background_color(m_selected ? theme.background_secondary_color : rgba(0, 0, 0, 0), transition);
    style(StyleType::HOVER).background_color(theme.background_secondary_color, transition);
    style(StyleType::ACTIVE).background_color(theme.background_secondary_color, transition);
}

EditorTextInput::EditorTextInput(std::string& value) : TextInputWidget(value) {}

void EditorTextInput::apply_theme_defaults(const Theme& theme) {
    TextInputWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 26.0F);

    if (auto* input = dynamic_cast<StyledNode*>(find("input"))) {
        const TransitionSpec transition(0.15F, easing::out_quad);

        input->configure_all_styles([&theme, &transition](Style& style) {
            style.padding({6.0F, 3.0F});
            style.border_radius(2.0F);
            style.background_color(theme.background_tertiary_color, transition);
        });

        input->style(StyleType::HOVER).background_color(theme.controls.hover_color, transition);
        input->style(StyleType::ACTIVE)
            .background_color(theme.controls.active_color, transition)
            .border_color(theme.accent_color, transition);

        input->style(StyleType::FOCUS).border_color(theme.accent_color, transition);
    }
}
