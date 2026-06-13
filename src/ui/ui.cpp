#include "ui.hpp"
#include "../game/game.hpp"
#include "../entity/player.hpp"
#include "helper.hpp"
#include "imgui.h"
#include "raylib.h"
#include "theme.hpp"

#include <rlImGui.h>
#include <string>

static constexpr ImGuiWindowFlags BASIC_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

static constexpr ImGuiWindowFlags POPUP_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;

static constexpr ImGuiChildFlags POPUP_CHILD_FLAGS = ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY;

void UI::initialize() {
    m_logo_texture = LoadTexture("resources/ui/logo.png");

    m_io = &ImGui::GetIO();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowPadding = ImVec2{0.0f, 0.0f};
    style.FramePadding = ImVec2{12.0f, 8.0f};
    style.ItemSpacing = ImVec2{10.0f, 10.0f};
    style.ItemInnerSpacing = ImVec2{8.0f, 6.0f};
    style.CellPadding = ImVec2{0.0f, 0.0f};

    m_io->IniFilename = NULL;
    m_io->LogFilename = NULL;

    ImFontConfig font_cfg;
    font_cfg.PixelSnapH = false;
    font_cfg.OversampleH = 5;
    font_cfg.OversampleV = 5;
    font_cfg.RasterizerMultiply = 1.2f;

    m_fonts[BALOO][FONT_SMALL] = m_io->Fonts->AddFontFromFileTTF("resources/fonts/Baloo-Regular.ttf", 16.0f, &font_cfg);
    m_fonts[BALOO][FONT_MEDIUM] =
        m_io->Fonts->AddFontFromFileTTF("resources/fonts/Baloo-Regular.ttf", 20.0f, &font_cfg);
    m_fonts[BALOO][FONT_LARGE] = m_io->Fonts->AddFontFromFileTTF("resources/fonts/Baloo-Regular.ttf", 26.0f, &font_cfg);
}

bool UI::render_level_button(std::string_view text, bool selected) {
    bool is_selected = false;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ui_theme::LEVEL_BUTTON_PADDING);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, ui_theme::LEVEL_BUTTON_BORDER_SIZE);

    ImGui::PushStyleColor(ImGuiCol_Button, ui_theme::TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ui_theme::TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ui_theme::TRANSPARENT_COLOR);

    ImGui::PushFont(m_fonts[BALOO][FONT_LARGE]);

    is_selected = ImGui::Button(text.data(), ui_theme::LEVEL_BUTTON_SIZE);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->Flags |= ImDrawListFlags_AntiAliasedLines;
    dl->Flags |= ImDrawListFlags_AntiAliasedFill;

    auto rect_min = ImGui::GetItemRectMin();
    auto rect_max = ImGui::GetItemRectMax();

    dl->AddRect({rect_min.x + 0.5f, rect_min.y + 0.5f}, {rect_max.x + 0.5f, rect_max.y + 0.5f},
                selected ? IM_COL32(0, 40, 200, 255) : IM_COL32(90, 90, 90, 200), 4.0f, 0, 2.0f);

    ImGui::PopFont();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    return is_selected;
}

bool UI::render_button(std::string_view text, ImVec2 padding, ImVec2 size) {
    bool is_selected = false;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, {20.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f});

    ImGui::PushFont(m_fonts[BALOO][FONT_MEDIUM]);

    is_selected = ImGui::Button(text.data(), size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->Flags |= ImDrawListFlags_AntiAliasedLines;
    dl->Flags |= ImDrawListFlags_AntiAliasedFill;

    auto rect_min = ImGui::GetItemRectMin();
    auto rect_max = ImGui::GetItemRectMax();

    dl->AddRect({rect_min.x + 0.5f, rect_min.y + 0.5f}, {rect_max.x + 0.5f, rect_max.y + 0.5f},
                ImGui::IsItemHovered() ? IM_COL32(0, 40, 200, 255) : IM_COL32(90, 90, 90, 200), 4.0f, 0, 2.0f);

    ImGui::PopFont();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);

    return is_selected;
}

void UI::render_main_menu() {
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
            // render logo
            float padding_v = available.y * 5.0f / 100.0f;

            ImGui::SetCursorPosY(padding_v);

            ui_helper::center_next_item_x((float)m_logo_texture.width);
            ImGui::Image((ImTextureID)m_logo_texture.id, {(float)m_logo_texture.width, (float)m_logo_texture.height});
            ImGui::Dummy({0.0f, ui_theme::MAIN_MENU_VERTICAL_PADDING});

            static std::string selected_level;

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

                    if (render_level_button(level->m_name, selected_level == level->m_file)) {
                        selected_level = level->m_file;
                    }

                    level_col++;
                }

                ImGui::EndTable();
            }
            ImGui::PopStyleVar(1);

            ImGui::Dummy({0.0f, ui_theme::MAIN_MENU_VERTICAL_PADDING});
            ui_helper::center_next_item_x(ui_theme::PLAY_BUTTON_SIZE.x);

            if (render_button("play", ui_theme::PLAY_BUTTON_PADDING, ui_theme::PLAY_BUTTON_SIZE) &&
                !selected_level.empty()) {
                game.load_level(selected_level, UIMode::PLAYFIELD);
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
            ImGui::PushFont(m_fonts[BALOO][FONT_SMALL]);

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

            if (debug_ui_pos.x < size.x) {
                debug_ui_pos = size;
            }

            ImGui::PopFont();
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
