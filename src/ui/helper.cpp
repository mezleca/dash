#include "helper.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

void ui_helper::center_next_item_x(float width, float padding) {
    float offset = (ImGui::GetContentRegionAvail().x - width + padding) * 0.5f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
}

void ui_helper::center_next_item_y(float height, float padding) {
    float offset = (ImGui::GetContentRegionAvail().y - height + padding) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
}

void ui_helper::center_text(std::string_view text, float padding) {
    float text_width = ImGui::CalcTextSize(text.data()).x + padding;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - text_width) * 0.5f);
    ImGui::TextUnformatted(text.data());
}

void ui_helper::center_next_window(float width, float height) {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2((display.x - width) * 0.5f, (display.y - height) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
}
