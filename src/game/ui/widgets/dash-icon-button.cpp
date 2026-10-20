#include "dash-icon-button.hpp"

using namespace ui;

DashIconButton::DashIconButton(Texture* icon) : ImageWidget(icon) {
    set_input_mode(InputMode::Target);
    set_size({px(30.0F), px(30.0F)});
    set_fit(ImageFit::Contain);
}

void DashIconButton::apply_theme_defaults(const Theme& theme) {
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

void DashIconButton::event(UiEvent& event) {
    if (event.type != EventType::Click || event.button != PointerButton::Left) {
        return;
    }

    if (m_on_click) {
        m_on_click();
    }

    event.stop_propagation();
}
