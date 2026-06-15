#include "pause.hpp"
#include "../theme.hpp"
#include "../../game/game.hpp"

PauseModal::PauseModal(UI* ui) : UIModal(ui, ui_modal_id::PAUSE, true) {
}

void PauseModal::on_remove() {
    if (game.m_current_level == nullptr) {
        return;
    }

    game.m_paused = false;
}

void PauseModal::render() {
    const ImVec2 available = m_ui->m_container_region;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, {20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.5f});
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    ImGui::SetCursorPos({0.0f, 0.0f});
    ImGui::BeginChild("##pause-container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        const char* text = "PAUSED";
        const int items_count = 3;
        const ImVec2 button_size = {200.0f, 32.0f};
        const float spacing = ImGui::GetStyle().ItemSpacing.y;

        ImGui::PushFont(m_ui->m_fonts[BALOO][FONT_MEDIUM]);

        const ImVec2 text_size = ImGui::CalcTextSize(text);
        const float items_height =
            (2.0f * button_size.y) + text_size.y + (static_cast<float>(items_count) - 1.0f) * spacing;
        const float y_start = (available.y - items_height) * 0.5f;
        const float button_x = (available.x - button_size.x) * 0.5f;

        ImGui::SetCursorPos({(available.x - text_size.x) * 0.5f, y_start});

        ImGui::TextUnformatted(text);

        ImGui::SetCursorPosX(button_x);
        if (m_ui->render_button("resume", ui_theme::BUTTON_PADDING, button_size)) {
            m_ui->remove_focused_modal();
        }

        ImGui::SetCursorPosX(button_x);
        if (m_ui->render_button("return to main menu", ui_theme::BUTTON_PADDING, button_size)) {
            game.unload_current_level();
        }

        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}
