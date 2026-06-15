#pragma once

#include "imgui.h"

namespace ui_theme {
    inline constexpr ImVec2 LEVEL_BUTTON_SIZE = ImVec2{300.0f, 200.0f};
    inline constexpr ImVec2 LEVEL_BUTTON_PADDING = ImVec2{8.0f, 6.0f};
    inline constexpr ImVec2 LEVEL_TABLE_PADDING = ImVec2{12.0f, 12.0f};
    inline constexpr ImVec2 BUTTON_SIZE = ImVec2{120.0f, 80.0f};
    inline constexpr ImVec2 BUTTON_PADDING = ImVec2{12.0f, 12.0f};

    inline constexpr float MAIN_MENU_VERTICAL_PADDING = 36.0f;
    inline constexpr float LEVEL_BUTTON_BORDER_SIZE = 0.0f;

    inline constexpr ImVec4 TRANSPARENT_COLOR = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};
    inline constexpr ImVec4 BG_COLOR = ImVec4{23.0f / 255.0f, 23.0f / 255.0f, 23.0f / 255.0f, 1.0f};
    inline constexpr ImVec4 NORMAL_BUTTON_COLOR = ImVec4{20.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f};
    inline constexpr ImVec4 SELECTED_BORDER_COLOR = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};

} // namespace ui_theme
