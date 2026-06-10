#include "ui.hpp"
#include "../game/game.hpp"
#include "../entity/player.hpp"

#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>

static constexpr ImGuiWindowFlags BASIC_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

static constexpr ImGuiWindowFlags POPUP_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;

static constexpr ImGuiChildFlags POPUP_CHILD_FLAGS = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY;

void UI::render_main_menu() {
    rlImGuiBegin();
    {
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
        ImGui::PopStyleVar(3);
    }
    rlImGuiEnd();
}

void UI::render_debug_ui() {
    rlImGuiBegin();
    {
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
    rlImGuiEnd();
}

void UI::render_playfield_ui() {
    // handle escape
    {
        static bool was_pressed = false;

        if (IsKeyPressed(KEY_ESCAPE) && !was_pressed) {
            m_is_pause_opened = !m_is_pause_opened;

            if (game.m_can_pause) {
                game.m_paused = m_is_pause_opened;
            }

            was_pressed = true;
        } else {
            was_pressed = false;
        }
    }

    rlImGuiBegin();
    {
        ImGuiStyle& style = ImGui::GetStyle();
        static const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // use the entire viewport
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.0f});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

        ImGui::Begin("##playfield_ui", nullptr, BASIC_WINDOW_FLAGS);
        {
            if (m_is_pause_opened) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, {20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.5f});
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

                ImGui::BeginChild("##pause_ui", viewport->WorkSize, ImGuiChildFlags_None, ImGuiWindowFlags_None);
                {
                    const char* label = "Press escape to unpause";

                    ImGui::SetCursorPos(
                        {viewport->WorkSize.x / 2.0f - ImGui::CalcTextSize(label).x, viewport->WorkSize.y / 2.0f});
                    ImGui::TextUnformatted(label);
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
    rlImGuiEnd();
}
