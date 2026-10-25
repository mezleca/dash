#pragma once

#include <imgui-ui/style/theme.hpp>
#include <imgui-ui/transition.hpp>

namespace ui {
    class Style;
    class StyledNode;
} // namespace ui

namespace dash_ui {
    inline constexpr float control_font_size = 26.0F;
    inline constexpr float spacing = 6.0F;
    inline constexpr float section_spacing = 12.0F;
    inline constexpr float panel_rounding = 4.0F;
    inline constexpr float panel_padding = 16.0F;

    inline constexpr ui::TransitionSpec control_transition{0.15F, ui::easing::out_cubic};
    inline constexpr ui::TransitionSpec panel_transition{0.2F, ui::easing::out_cubic};

    void panel_style(ui::Style& style, const ui::Theme& theme);
    void editor_panel_style(ui::Style& style, const ui::Theme& theme);
    void input_style(ui::StyledNode& input, const ui::Theme& theme);
    void card_style(ui::StyledNode& card, const ui::Theme& theme);
} // namespace dash_ui

ui::Theme make_dash_theme();
