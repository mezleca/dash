#include "level.hpp"
#include "../theme.hpp"
#include "../../utils/math.hpp"
#include "../../game/game.hpp"
#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdint>

int64_t get_time_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

LevelSelectorModal::LevelSelectorModal(UI* ui) : UIModal(ui, ui_modal_id::LEVEL_SELECTOR) {
}

void LevelSelectorModal::on_remove() {
}

void LevelSelectorModal::on_escape() {
    m_ui->remove_focused_modal();
}

void LevelSelectorModal::render() {
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const int levels_count = static_cast<int>(game.m_levels.size());

    static constexpr float DRAG_THRESHOLD = 8.0f;

    const bool mouse_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    const bool mouse_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    const bool mouse_released = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    if (mouse_pressed) {
        m_holding_start_x = GetMousePosition().x;
        m_holding_offset_x = 0.0f;
        m_mouse_held = true;
        m_is_dragging = false;
    }

    if (m_mouse_held && mouse_down) {
        m_holding_offset_x = GetMousePosition().x - m_holding_start_x;
        if (std::fabs(m_holding_offset_x) >= DRAG_THRESHOLD) {
            m_is_dragging = true;
        }
    }

    if (mouse_released) {
        m_mouse_held = false;
        m_is_dragging = false;
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui_theme::BG_COLOR);
    ImGui::BeginChild("##level-carousel", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        int64_t cur_time_ms = get_time_ms();

        const ImVec2 button_size = {available.x * 50.0f / 100.0f, available.y * 50.0f / 100.0f};
        const float start_x = (available.x - button_size.x) * 0.5f;
        const float start_y = (available.y - button_size.y) * 0.5f;
        const float item_stride = button_size.x + 20.0f;

        if (m_target_pos_x != m_current_pos_x) {
            m_current_pos_x = d_math::lerp(m_current_pos_x, m_target_pos_x, 0.12f);
        }

        for (size_t i = 0; i < game.m_levels.size(); i++) {
            DashLevel* level = game.m_levels.at(i).get();

            float item_base_x = item_stride * static_cast<float>(i + 1);

            if (m_target_pos_x == 0.0f || static_cast<int>(i) == m_focused_item) {
                m_target_pos_x = item_base_x - m_holding_offset_x;
                if (static_cast<int>(i) != m_focused_item) {
                    m_current_pos_x = item_base_x - m_holding_offset_x;
                }
            }

            bool snapped = false;

            // snap to next or previous item on drag release
            if (mouse_released && std::fabs(m_holding_offset_x) >= item_stride / 4.0f) {
                int direction = m_holding_offset_x < 0.0f ? 1 : -1;

                m_focused_item = std::clamp(m_focused_item + direction, 0, levels_count - 1);
                m_target_pos_x = (item_stride * static_cast<float>(m_focused_item + 1)) - m_holding_offset_x;

                // reset draw offset
                m_holding_offset_x = 0.0f;
                m_holding_start_x = 0.0f;

                m_last_snap_ms = cur_time_ms;
                snapped = true;
            }

            float screen_x = start_x + item_base_x - m_current_pos_x;

            ImGui::SetCursorPosX(screen_x);
            ImGui::SetCursorPosY(start_y);

            std::string button_id = level->m_name + std::to_string(i);

            bool clicked = m_ui->render_level_button(level->m_name.c_str(), button_id, button_size,
                                                     m_focused_item == static_cast<int>(i));

            // only handle as click if the mouse didn't drag
            if (clicked && !m_is_dragging && !snapped) {
                // ignore input if we just finished snapping
                if (m_last_snap_ms != 0 && cur_time_ms - m_last_snap_ms < 100) {
                    continue;
                }

                if (m_focused_item != static_cast<int>(i)) {
                    m_focused_item = static_cast<int>(i);
                    m_target_pos_x = item_base_x;
                } else {
                    if (game.load_level(level->m_file.c_str())) {
                        game.start_level(true);
                    }
                }
            }
        }

        // render back button
        {
            ImVec2 size = {48.0f, 32.0f};
            float spacing = 12.0f;

            ImGui::SetCursorPos({10.0f, available.y - size.y - spacing});

            if (m_ui->render_button("back", {}, size)) {
                m_ui->remove_focused_modal();
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
}
