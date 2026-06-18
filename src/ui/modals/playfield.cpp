#include "playfield.hpp"
#include "../../game/game.hpp"
#include "../theme.hpp"
#include "imgui.h"

PlayfieldModal::PlayfieldModal(UI* ui) : UIModal(ui, ui_modal_id::PLAYFIELD) {
}

void PlayfieldModal::on_remove() {
    m_mode = PlayfieldMode::NONE;
}

void PlayfieldModal::on_escape() {
    bool can_pause =
        m_mode != PlayfieldMode::PAUSE && m_mode != PlayfieldMode::DEATH && m_mode != PlayfieldMode::FINISH;

    if (can_pause) {
        m_mode = PlayfieldMode::PAUSE;
        game.m_paused = true;
    } else if (m_mode == PlayfieldMode::PAUSE) {
        m_mode = PlayfieldMode::NONE;
        game.m_paused = false;
    }
}

void PlayfieldModal::show_default_screen() {
    const ImVec2 available = m_ui->m_container_region;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ui_theme::TRANSPARENT_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    ImGui::SetCursorPos({0.0f, 0.0f});
    ImGui::BeginChild("##default-screen", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        ImVec2 progress_size = {128.0f, 16.0f};
        ImVec2 progress_pos = {(available.x - progress_size.x) * 0.5f, 10.0f};

        m_ui->render_progress_bar("", game.m_current_level->m_current_progress, 100.0f, progress_pos, progress_size);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}

void PlayfieldModal::show_pause_screen() {
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
            game.unload_current_level(true);
        }

        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}

void PlayfieldModal::show_death_screen() {
    const ImVec2 available = m_ui->m_container_region;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, {20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.5f});
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    ImGui::SetCursorPos({0.0f, 0.0f});
    ImGui::BeginChild("##death-container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
    {
        const char* text = "YOU DIED";
        const int items_count = 3;
        const ImVec2 button_size = {160.0f, 32.0f};
        const float spacing = ImGui::GetStyle().ItemSpacing.y;

        ImGui::PushFont(m_ui->m_fonts[BALOO][FONT_MEDIUM]);

        const ImVec2 text_size = ImGui::CalcTextSize(text);
        const float items_height =
            (2.0f * button_size.y) + text_size.y + (static_cast<float>(items_count) - 1.0f) * spacing;
        const float y_start = (available.y - items_height) * 0.5f;
        const float x_start = (available.x - button_size.x) * 0.5f;

        ImGui::SetCursorPos({(available.x - text_size.x) * 0.5f, y_start});
        ImGui::TextUnformatted(text);

        ImGui::SetCursorPosX(x_start);

        if (m_ui->render_button("retry", ui_theme::BUTTON_PADDING, button_size)) {
            game.restart_current_level();
        }

        ImGui::SetCursorPosX(x_start);

        if (m_ui->render_button("main menu", ui_theme::BUTTON_PADDING, button_size)) {
            game.unload_current_level(true);
        }

        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
}

void PlayfieldModal::show_finish_screen() {
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
            game.unload_current_level(true);
        }

        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

void PlayfieldModal::render() {
    show_default_screen();

    switch (m_mode) {
        case PlayfieldMode::PAUSE:
            show_pause_screen();
            break;
        case PlayfieldMode::DEATH:
            show_death_screen();
            break;
        case PlayfieldMode::FINISH:
            show_finish_screen();
            break;
        default:
            break;
    }
}
