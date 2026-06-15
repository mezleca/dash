#include "finish.hpp"
#include "../../game/game.hpp"
#include "../theme.hpp"

FinishModal::FinishModal(UI* ui) : UIModal(ui, ui_modal_id::FINISH) {
}

void FinishModal::render() {
    const ImVec2 available = m_ui->m_container_region;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, {20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.5f});
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    ImGui::SetCursorPos({0.0f, 0.0f});
    ImGui::BeginChild("##finish-container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        const char* text = "LEVEL COMPLETED";
        const ImVec2 button_size = {160.0f, 32.0f};
        const float spacing = ImGui::GetStyle().ItemSpacing.y;

        ImGui::PushFont(m_ui->m_fonts[BALOO][FONT_MEDIUM]);

        const ImVec2 text_size = ImGui::CalcTextSize(text);
        const float items_height = button_size.y + text_size.y + spacing;
        const float y_start = (available.y - items_height) * 0.5f;
        const float x_start = (available.x - button_size.x) * 0.5f;

        ImGui::SetCursorPos({(available.x - text_size.x) * 0.5f, y_start});
        ImGui::TextUnformatted(text);

        ImGui::SetCursorPosX(x_start);

        if (m_ui->render_button("main menu", ui_theme::BUTTON_PADDING, button_size)) {
            game.unload_current_level();
        }

        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}
