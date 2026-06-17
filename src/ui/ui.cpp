#include "ui.hpp"
#include "theme.hpp"

#include <algorithm>
#include <rlImGui.h>

static constexpr ImGuiWindowFlags BASIC_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

void UI::initialize() {
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

    m_io->IniFilename = nullptr;
    m_io->LogFilename = nullptr;

    ImFontConfig font_cfg;
    font_cfg.PixelSnapH = false;
    font_cfg.OversampleH = 5;
    font_cfg.OversampleV = 5;
    font_cfg.RasterizerMultiply = 1.2f;

    m_fonts[BALOO][FONT_SMALL] = m_io->Fonts->AddFontFromFileTTF("resources/fonts/Baloo-Regular.ttf", 16.0f, &font_cfg);
    m_fonts[BALOO][FONT_MEDIUM] =
        m_io->Fonts->AddFontFromFileTTF("resources/fonts/Baloo-Regular.ttf", 20.0f, &font_cfg);
    m_fonts[BALOO][FONT_LARGE] = m_io->Fonts->AddFontFromFileTTF("resources/fonts/Baloo-Regular.ttf", 26.0f, &font_cfg);

    m_debug_modal = new DebugModal(this);
    m_level_selector_modal = new LevelSelectorModal(this);
    m_menu_modal = new MenuModal(this);
    m_playfield_modal = new PlayfieldModal(this);

    show_modal(m_menu_modal);
}

bool UI::is_modal_focused(UIModal* modal) const {
    if (m_modals.empty()) {
        return false;
    }

    return m_modals.back() == modal;
}

bool UI::has_modal(std::string_view id) const {
    return std::any_of(m_modals.begin(), m_modals.end(), [id](const UIModal* modal) { return modal->m_id == id; });
}

UIModal* UI::focused_modal() const {
    if (m_modals.empty()) {
        return nullptr;
    }

    return m_modals.back();
}

void UI::show_modal(UIModal* modal, bool wipe) {
    if (modal == nullptr) {
        return;
    }

    if (wipe) {
        clear_modals();
    }

    m_modals.push_back(modal);
}

bool UI::remove_modal(std::string_view id) {
    bool removed = false;

    for (auto it = m_modals.begin(); it != m_modals.end();) {
        UIModal* modal = *it;

        if (modal->m_id != id) {
            it++;
            continue;
        }

        modal->on_remove();
        it = m_modals.erase(it);
        removed = true;
    }

    return removed;
}

bool UI::remove_focused_modal() {
    UIModal* modal = focused_modal();

    if (modal == nullptr) {
        return false;
    }

    modal->on_remove();
    m_modals.pop_back();
    return true;
}

void UI::clear_modals() {
    for (UIModal* modal : m_modals) {
        modal->on_remove();
    }

    m_modals.clear();
}

void UI::handle_escape() {
    if (!IsKeyPressed(KEY_ESCAPE)) {
        return;
    }

    UIModal* modal = focused_modal();

    if (modal != nullptr) {
        modal->on_escape();
        return;
    }
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
                selected || ImGui::IsItemHovered() ? IM_COL32(0, 40, 200, 255) : IM_COL32(90, 90, 90, 200), 4.0f, 0,
                2.0f);

    ImGui::PopFont();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    return is_selected;
}

bool UI::render_button(std::string_view text, ImVec2 padding, ImVec2 size) {
    bool is_selected = false;
    const ImVec2 frame_padding = {padding.x, 0.0f};

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, frame_padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.5f, 0.5f});

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
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(3);

    return is_selected;
}

bool UI::render_menu_button(std::string_view text, ImVec2 padding, ImVec2 size) {
    bool is_selected = false;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, ui_theme::TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ui_theme::TRANSPARENT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ui_theme::TRANSPARENT_COLOR);

    ImGui::PushFont(m_fonts[BALOO][FONT_MEDIUM]);

    is_selected = ImGui::Button(text.data(), size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->Flags |= ImDrawListFlags_AntiAliasedLines;
    dl->Flags |= ImDrawListFlags_AntiAliasedFill;

    auto rect_size = ImGui::GetItemRectSize();
    auto rect_min = ImGui::GetItemRectMin();

    dl->AddLine({rect_min.x + 0.5f, rect_min.y + 0.5f + rect_size.y + 2.0f},
                {rect_min.x + 0.5f + rect_size.x, rect_min.y + 0.5f + rect_size.y + 2.0f},
                ImGui::IsItemHovered() ? IM_COL32(0, 40, 200, 255) : IM_COL32(90, 90, 90, 200), 2.0f);

    ImGui::PopFont();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);

    return is_selected;
}

void UI::render() {
    static const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

    ImGui::Begin("##ui", nullptr, BASIC_WINDOW_FLAGS);
    {
        m_container_region = ImGui::GetContentRegionAvail();

        for (UIModal* modal : m_modals) {
            ImGui::SetCursorPos({0.0f, 0.0f});
            modal->render();
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(3);
}
