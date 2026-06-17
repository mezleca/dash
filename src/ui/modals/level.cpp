#include "level.hpp"
#include "../theme.hpp"
#include "../../game/game.hpp"
#include "imgui.h"

LevelSelectorModal::LevelSelectorModal(UI* ui) : UIModal(ui, ui_modal_id::LEVEL_SELECTOR) {
}

void LevelSelectorModal::on_remove() {
}

void LevelSelectorModal::on_escape() {
    m_ui->remove_focused_modal();
}

void LevelSelectorModal::render() {
    const ImVec2 available = ImGui::GetContentRegionAvail();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui_theme::BG_COLOR);

    ImGui::BeginChild("##container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        const int levels_count = static_cast<int>(game.m_levels.size());
        const int table_columns_count = levels_count + 2;

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ui_theme::LEVEL_TABLE_PADDING);

        if (ImGui::BeginTable("levels", table_columns_count, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("##left-padding", ImGuiTableColumnFlags_WidthStretch);

            for (int column = 0; column < levels_count; column++) {
                ImGui::TableSetupColumn("##level", ImGuiTableColumnFlags_WidthFixed, ui_theme::LEVEL_BUTTON_SIZE.x);
            }

            ImGui::TableSetupColumn("##right-padding", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();

            int level_col = 1;

            for (const auto& level_it : game.m_levels) {
                ImGui::TableSetColumnIndex(level_col);

                DashLevel* level = level_it.second;

                if (m_ui->render_level_button(level->m_name, false)) {
                    if (game.load_level(level->m_file.c_str())) {
                        game.start_level(true);
                    }
                }

                level_col++;
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleVar(1);

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
