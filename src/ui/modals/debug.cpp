#include "debug.hpp"
#include "../../entity/player.hpp"
#include "../../game/game.hpp"

DebugModal::DebugModal(UI* ui) : UIModal(ui, ui_modal_id::DEBUG) {
}

void DebugModal::on_remove() {
}

void DebugModal::on_escape() {
}

void DebugModal::render() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 debug_ui_pos = {0, 0};

    ImGui::PushStyleColor(ImGuiCol_ChildBg, {10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 0.0f});

    ImGui::BeginChild("##debug_info", {0, 0}, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
                      ImGuiWindowFlags_None);
    {
        ImGui::PushFont(m_ui->m_fonts[BALOO][FONT_MEDIUM]);

        ImGui::Text("FPS: %i", GetFPS());
        ImGui::Text("Frametime: %f", GetFrameTime());

        if (game.m_player != nullptr) {
            ImGui::Separator();
            ImGui::Text("Player pos: (%.2f, %.2f)", game.m_player->position.x, game.m_player->position.y);
            ImGui::Text("Player vel: (%.2f, %.2f)", game.m_player->velocity.x, game.m_player->velocity.y);
        }

        if (game.m_current_level != nullptr) {
            ImGui::Separator();
            ImGui::Text("Loaded level: %s", game.m_current_level->m_name.c_str());
            ImGui::Text("Level progress: %f", game.m_current_level->m_current_progress);
        }

        ImGui::Separator();

        UIModal* focused_modal = game.m_ui.focused_modal();

        if (focused_modal != nullptr) {
            ImGui::Text("Active modal: %s", focused_modal->m_id.c_str());
        }

        ImGui::Text("Modal count: %zu", game.m_ui.modal_count());

        auto size = ImGui::GetWindowSize();

        if (debug_ui_pos.x < size.x) {
            debug_ui_pos = size;
        }

        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
}
