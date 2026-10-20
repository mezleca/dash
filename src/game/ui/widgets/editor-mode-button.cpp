#include "editor-mode-button.hpp"
#include <imgui-ui/surface.hpp>

using namespace ui;

EditorModeButton::EditorModeButton(Texture* icon, bool selected) : ImageWidget(icon), m_selected(selected) {
    set_input_mode(InputMode::Target);
    set_size({px(38.0F), px(38.0F)});
    set_fit(ImageFit::Contain);
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

void EditorModeButton::event(UiEvent& event) {
    if (event.type != EventType::Click || event.button != PointerButton::Left) {
        return;
    }

    if (m_on_click) {
        m_on_click();
    }

    event.stop_propagation();
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
