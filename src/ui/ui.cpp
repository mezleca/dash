#include "ui.hpp"
#include "../game/game.hpp"

#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

void UI::render_main_menu() {
    rlImGuiBegin();
    {
        ImGuiStyle& style = ImGui::GetStyle();

        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        static const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // use the entire viewport
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("##main-menu", nullptr, flags);
        {
            const ImVec2 available = ImGui::GetContentRegionAvail();

            ImGui::BeginChild("##container", available, ImGuiChildFlags_None, ImGuiWindowFlags_None);
            {
                float size = ImGui::CalcTextSize("play").x + style.FramePadding.x * 2.0f;
                float off = (available.x - size) * 0.5f;

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

                static const char* selected_level;
                static const char* selected_level_label;

                // TODO: scrollable layout instead of a single combo box
                if (ImGui::BeginCombo("level", selected_level_label)) {
                    for (const auto& level_it : game.m_levels) {
                        auto* level = level_it.second;

                        const char* level_id = level->m_file.c_str();
                        const char* level_name = level->m_name.c_str();

                        ImGui::PushID(level_id);

                        if (ImGui::Selectable(level_name, selected_level == level_id,
                                              ImGuiSelectableFlags_SelectOnNav)) {
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
                    if (selected_level != nullptr) {
                        game.load_level(selected_level, UIMode::PLAYFIELD);
                    }
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }
    rlImGuiEnd();
}
