#include "dash-text-input.hpp"

#include <imgui-ui/style/styled-node.hpp>
#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

DashTextInput::DashTextInput(std::string& value, std::string id) : TextInputWidget(value, std::move(id)) {}

void DashTextInput::apply_theme_defaults(const Theme& theme) {
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
