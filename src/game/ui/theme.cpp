#include "theme.hpp"

using namespace ui;

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
