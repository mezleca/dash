#include "menu.hpp"
#include "../../game/game.hpp"
#include "../../ui/helper.hpp"
#include "../../ui/theme.hpp"

#include "level.hpp"

MenuModal::MenuModal(UI* ui) : UIModal(ui, ui_modal_id::MENU) {
    m_logo_texture = LoadTexture("resources/ui/logo.png");
}

void MenuModal::on_remove() {
}

void MenuModal::on_escape() {
}

void MenuModal::render() {
    const ImVec2 available = ImGui::GetContentRegionAvail();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui_theme::BG_COLOR);
    ImGui::BeginChild("##menu-container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        // render logo
        {
            float padding_v = available.y * 25.0f / 100.0f;
            ImGui::SetCursorPosY(padding_v);

            ui_helper::center_next_item_x((float)m_logo_texture.width);

            ImGui::Image((ImTextureID)m_logo_texture.id, {(float)m_logo_texture.width, (float)m_logo_texture.height});
            ImGui::Dummy({0.0f, ui_theme::MAIN_MENU_VERTICAL_PADDING});
        }

        // render menu buttons
        {
            float available_width = ImGui::GetContentRegionAvail().x;

            ImVec2 button_size = {150.0f, 32.0f};
            int num_buttons = 3;

            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float width = (num_buttons * button_size.x) + (num_buttons - 1) * spacing;
            float start = (available_width - width) * 0.5f;

            ImGui::SetCursorPosX(start);

            if (m_ui->render_menu_button("play", ui_theme::BUTTON_PADDING, button_size)) {
                m_ui->show_modal(m_ui->m_level_selector_modal);
            }

            ImGui::SameLine();

            if (m_ui->render_menu_button("settings", ui_theme::BUTTON_PADDING, button_size)) {
            }

            ImGui::SameLine();

            if (m_ui->render_menu_button("exit", ui_theme::BUTTON_PADDING, button_size)) {
                game.m_finished = true;
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
}
