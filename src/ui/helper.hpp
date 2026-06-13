#pragma once

#include <string_view>

namespace ui_helper {
    // imgui helpers
    void center_next_item_x(float width, float padding = 0.0f);
    void center_next_item_y(float width, float padding = 0.0f);
    void center_text(std::string_view text, float padding = 0.0f);
    void center_next_window(float width, float height);
} // namespace ui_helper
