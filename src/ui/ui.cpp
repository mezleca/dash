#include "ui.hpp"
#include "../game/game.hpp"
#include "../entity/player.hpp"

#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>
#include <string>

static constexpr ImGuiWindowFlags BASIC_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

static constexpr ImGuiWindowFlags POPUP_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;

static constexpr ImGuiChildFlags POPUP_CHILD_FLAGS = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY;

void UI::render_main_menu() {
    ImGuiStyle& style = ImGui::GetStyle();

    static const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // use the entire viewport
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

    ImGui::Begin("##main-menu", nullptr, BASIC_WINDOW_FLAGS);
    {
        const ImVec2 available = ImGui::GetContentRegionAvail();

        ImGui::BeginChild("##container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
        {
            float size = ImGui::CalcTextSize("play").x + style.FramePadding.x * 2.0f;
            float off = (available.x - size) * 0.5f;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

            static std::string selected_level;
            static std::string selected_level_label = "select level";

            (void)game.m_player;

            if (ImGui::BeginCombo("level", selected_level_label.c_str())) {
                for (const auto& level_it : game.m_levels) {
                    auto* level = level_it.second;

                    const std::string& level_id = level_it.first;
                    const char* level_name = level->m_name.c_str();

                    ImGui::PushID(level_id.c_str());

                    if (ImGui::Selectable(level_name, selected_level == level_id, ImGuiSelectableFlags_SelectOnNav)) {
                        selected_level = level_id;
                        selected_level_label = level_name;
                    }

                    if (selected_level == level_id) {
                        ImGui::SetItemDefaultFocus();
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("play")) {
                if (!selected_level.empty()) {
                    game.load_level(selected_level, UIMode::PLAYFIELD);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void UI::render_debug_ui() {
    ImGuiStyle& style = ImGui::GetStyle();

    static const ImGuiViewport* viewport = ImGui::GetMainViewport();
    static ImVec2 debug_ui_pos = {0, 0};

    ImGui::SetNextWindowPos({viewport->WorkSize.x - debug_ui_pos.x - 10.0f, 0.0f});
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.0f});

    ImGui::Begin("##debug_ui", nullptr, BASIC_WINDOW_FLAGS);
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
        ImGui::BeginChild("##debug_info", {0, 0}, POPUP_CHILD_FLAGS, ImGuiWindowFlags_None);
        {
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

            auto size = ImGui::GetWindowSize();

            if (debug_ui_pos.x < size.x) debug_ui_pos = size;
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(1);
    }

    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(1);
}

void UI::render_playfield_ui() {
    ImGuiStyle& style = ImGui::GetStyle();
    static const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // use the entire viewport
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.0f});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

    ImGui::Begin("##playfield_ui", &m_playfield_container_open, BASIC_WINDOW_FLAGS);
    {
        bool show_container = m_show_pause || m_show_dead || m_show_finished;

        if (show_container) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, {20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.5f});
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

            ImGui::BeginChild("##container", viewport->WorkSize, ImGuiChildFlags_None, ImGuiWindowFlags_None);
            {
                if (m_show_pause) {
                    static const char* label = "Press escape to unpause";

                    ImGui::SetCursorPos(
                        {viewport->WorkSize.x / 2.0f - ImGui::CalcTextSize(label).x, viewport->WorkSize.y / 2.0f});
                    ImGui::TextUnformatted(label);
                } else if (m_show_dead) {
                    static const char* label = "Press R to reset";

                    ImGui::SetCursorPos(
                        {viewport->WorkSize.x / 2.0f - ImGui::CalcTextSize(label).x, viewport->WorkSize.y / 2.0f});
                    ImGui::TextUnformatted(label);
                } else {
                    static const std::string menu_label = "Return to main menu";
                    const std::string label = "You finished " + std::string(game.m_current_level->m_name);

                    ImGui::SetCursorPos({viewport->WorkSize.x / 2.0f - ImGui::CalcTextSize(label.c_str()).x,
                                         viewport->WorkSize.y / 2.0f});
                    ImGui::TextUnformatted(label.c_str());

                    ImGui::SetCursorPosX(viewport->WorkSize.x / 2.0f - ImGui::CalcTextSize(menu_label.c_str()).x);

                    if (ImGui::Button(menu_label.c_str())) {
                        game.unload_current_level();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(1);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(1);
}
