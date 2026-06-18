#include "level.hpp"
#include "../theme.hpp"
#include "../../utils/math.hpp"
#include "../../game/game.hpp"
#include "raylib.h"
#include <cmath>
#include <iostream>

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

    static int focused_item = 0;
    static float holding_offset_x = 0.0f;
    static float holding_start_x = 0.0f;
    static float target_pos_x = 0.0f;
    static float current_pos_x = 0.0f;
    static bool mouse_held = false;
    static bool is_dragging = false;

    static constexpr float DRAG_THRESHOLD = 8.0f;

    const bool mouse_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    const bool mouse_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    const bool mouse_released = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    if (mouse_pressed) {
        holding_start_x = GetMousePosition().x;
        holding_offset_x = 0.0f;
        mouse_held = true;
        is_dragging = false;
    }

    if (mouse_held && mouse_down) {
        holding_offset_x = GetMousePosition().x - holding_start_x;
        if (std::fabs(holding_offset_x) >= DRAG_THRESHOLD) {
            is_dragging = true;
        }
    }

    if (mouse_released) {
        mouse_held = false;
        is_dragging = false;
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui_theme::BG_COLOR);
    ImGui::BeginChild("##level-carousel", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        const ImVec2 button_size = {available.x * 50.0f / 100.0f, available.y * 50.0f / 100.0f};
        const float start_x = (available.x - button_size.x) * 0.5f;
        const float start_y = (available.y - button_size.y) * 0.5f;
        const float item_stride = button_size.x + 20.0f;

        if (target_pos_x != current_pos_x) {
            current_pos_x = d_math::lerp(current_pos_x, target_pos_x, 0.12f);
        }

        for (size_t i = 0; i < game.m_levels.size(); i++) {
            DashLevel* level = game.m_levels.at(i);

            float item_base_x = item_stride * (i + 1);

            if (target_pos_x == 0.0f || i == focused_item) {
                target_pos_x = item_base_x - holding_offset_x;
                if (i != focused_item) {
                    current_pos_x = item_base_x - holding_offset_x;
                }
            }

            bool snapped = false;

            // snap to next or previous item on drag release
            if (mouse_released && std::fabs(holding_offset_x) >= item_stride / 4.0f) {
                int direction = holding_offset_x < 0.0f ? 1 : -1;

                focused_item = std::clamp(focused_item + direction, 0, levels_count - 1);
                target_pos_x = (item_stride * (focused_item + 1)) - holding_offset_x;

                // reset draw offset
                holding_offset_x = 0.0f;
                holding_start_x = 0.0f;

                snapped = true;
            }

            float screen_x = start_x + item_base_x - current_pos_x;

            ImGui::SetCursorPosX(screen_x);
            ImGui::SetCursorPosY(start_y);

            std::string button_id = level->m_name + std::to_string(i);

            bool clicked = m_ui->render_level_button(level->m_name.c_str(), button_id, button_size,
                                                     focused_item == static_cast<int>(i));

            // only handle as click if the mouse didn't drag
            if (clicked && !is_dragging && !snapped) {
                if (focused_item != static_cast<int>(i)) {
                    focused_item = static_cast<int>(i);
                    target_pos_x = item_base_x;
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
