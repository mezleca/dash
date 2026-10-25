#include "theme.hpp"
#include <imgui-ui/style/styled-node.hpp>

using namespace ui;

void dash_ui::panel_style(Style& style, const Theme& theme) {
    style.background_color(theme.background_secondary_color);
    style.padding({panel_padding, panel_padding});
    style.border_radius(panel_rounding);
    style.border(BorderEdge{2.0F, BorderType::Solid});
    style.border_color(rgba(149, 204, 221, 60));
}

void dash_ui::editor_panel_style(Style& style, const Theme& theme) {
    panel_style(style, theme);

    style.border(BorderEdge{1.0F, BorderType::Solid});
    style.padding({spacing, spacing});
}

void dash_ui::input_style(StyledNode& input, const Theme& theme) {
    const auto& transition = control_transition;

    input.configure_all_styles([&](Style& style) {
        style.padding({6.0F, 3.0F});
        style.border_radius(theme.controls.rounding);
        style.background_color(theme.background_tertiary_color, transition);
    });

    input.style(StyleType::HOVER).background_color(theme.controls.hover_color, transition);
    input.style(StyleType::ACTIVE)
        .background_color(theme.controls.active_color, transition)
        .border_color(theme.accent_color, transition);
    input.style(StyleType::FOCUS).border_color(theme.accent_color, transition);
}

void dash_ui::card_style(StyledNode& card, const Theme& theme) {
    const auto& transition = control_transition;

    card.configure_all_styles([&](Style& style) {
        panel_style(style, theme);
        style.background_color(theme.background_secondary_color, transition);
        style.border_color(theme.controls.border_color, transition);
    });

    card.style(StyleType::HOVER)
        .background_color(theme.controls.background_color, transition)
        .border_color(theme.accent_hover_color, transition);
    card.style(StyleType::ACTIVE).background_color(theme.controls.active_color, transition);
}

Theme make_dash_theme() {
    Theme theme;

    theme.accent_color = rgb(60, 117, 232);
    theme.accent_hover_color = rgb(81, 151, 251);
    theme.background_color = rgb(26, 27, 31);
    theme.background_secondary_color = rgb(36, 38, 46);
    theme.background_tertiary_color = rgb(26, 27, 31);
    theme.header_background_color = rgb(26, 27, 31);
    theme.text_color = rgb(208, 231, 230);
    theme.text_secondary_color = rgb(149, 204, 221);
    theme.border_color = rgb(149, 204, 221);
    theme.header_border_color = rgb(149, 204, 221);
    theme.button_active_color = rgb(66, 116, 217);

    theme.controls.background_color = rgb(52, 55, 61);
    theme.controls.hover_color = rgb(77, 143, 219);
    theme.controls.active_color = rgb(69, 127, 245);
    theme.controls.border_color = rgb(92, 92, 92);
    theme.controls.mark_color = rgb(208, 231, 230);
    theme.controls.rounding = 3.0F;
    theme.controls.font_size = dash_ui::control_font_size;
    theme.controls.thumb_size = 10.0F;

    theme.scrollbar.size = 8.0F;
    theme.scrollbar.rounding = 4.0F;
    theme.scrollbar.minimum_grab_size = 8.0F;
    theme.scrollbar.grab_rounding = 4.0F;
    theme.scrollbar.background_color = rgba(0, 0, 0, 0);
    theme.scrollbar.grab_color = rgb(60, 117, 232);
    theme.scrollbar.grab_hovered_color = rgb(34, 73, 153);
    theme.scrollbar.grab_active_color = rgb(34, 73, 153);

    theme.metrics.window_rounding = 4.0F;
    theme.metrics.child_rounding = 4.0F;
    theme.metrics.popup_rounding = 4.0F;
    theme.metrics.tab_rounding = 3.0F;
    theme.metrics.frame_border_size = 0.0F;
    theme.metrics.window_padding = {4.0F, 4.0F};
    theme.metrics.cell_padding = {4.0F, 3.0F};
    theme.metrics.frame_padding = {6.0F, 4.0F};
    theme.metrics.item_spacing = {4.0F, 4.0F};
    theme.metrics.item_inner_spacing = {4.0F, 3.0F};

    return theme;
}
