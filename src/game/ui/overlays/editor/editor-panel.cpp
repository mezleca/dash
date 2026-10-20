#include "editor.hpp"
#include "game/dash.hpp"
#include "game/editor/editor.hpp"
#include "game/ui/widgets/dash-icon-button.hpp"
#include "game/ui/widgets/editor-mode-button.hpp"
#include "game/ui/widgets/editor-properties-panel.hpp"
#include "game/ui/widgets/editor-tab-button.hpp"

#include <utility>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/layout/container.hpp>

using namespace ui;

EditorPanel::EditorPanel(Dash& game, InputCallback on_key_press)
    : LayerContainer("editor-panel"), m_game(game), m_on_key_press(std::move(on_key_press)) {
    set_size({grow(), grow()});
    set_direction(StackDirection::Vertical);
    set_spacing(6.0F);
    set_input_mode(InputMode::None);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({6.0F, 6.0F});
    });

    auto& mode_row = add<Container>("editor-mode-row", StackDirection::Horizontal);
    mode_row.set_size({grow(), fit()});
    mode_row.set_input_mode(InputMode::None);

    auto& mode_panel = mode_row.add<Container>("editor-mode-panel", StackDirection::Horizontal);
    mode_panel.set_size({fit(), fit()});
    mode_panel.set_spacing(2.0F);
    mode_panel.configure_all_styles([](Style& style) {
        style.background_color(rgba(40, 42, 47, 220));
        style.border(BorderEdge{1.0F, BorderType::Solid});
        style.border_color(rgba(149, 204, 221, 60));
        style.border_radius(4.0F);
        style.padding({2.0F, 2.0F});
    });

    auto* pointer_icon =
        m_game.surface().runtime().textures().add("editor-pointer", RESOURCES_LOCATION / "ui/editor/pointer.svg");
    auto* hand_icon = m_game.surface().runtime().textures().add("editor-hand", RESOURCES_LOCATION / "ui/editor/hand.svg");

    m_select_button = &mode_panel.add<EditorModeButton>(pointer_icon, true);
    m_select_button->on_click([this] { set_mode(EditorMode::Select); });

    m_move_button = &mode_panel.add<EditorModeButton>(hand_icon, false);
    m_move_button->on_click([this] { set_mode(EditorMode::Move); });

    auto* close_icon = m_game.surface().runtime().textures().add("editor-close", RESOURCES_LOCATION / "ui/close.svg");
    auto& close = mode_row.add<DashIconButton>(close_icon);
    close.set_anchor(Anchor::TopRight);
    close.on_click([this] {
        if (auto* editor = m_game.editor()) {
            editor->clear_selection();
        }
        m_game.return_to_menu();
    });

    auto& workspace = add<Container>("editor-workspace", StackDirection::Horizontal);
    workspace.set_size({grow(), percent(60.0F)});
    workspace.set_spacing(6.0F);
    workspace.set_input_mode(InputMode::None);

    workspace.add<PropertiesPanel>(game);

    m_game_space = &workspace.add<Container>("editor-game-space");
    m_game_space->set_size({grow(), grow()});
    m_game_space->set_input_mode(InputMode::None);
    m_game_space->configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({0.0F, 0.0F});
    });

    auto& bottom = add<Container>("editor-bottom");
    bottom.set_size({grow(), grow()});
    bottom.set_spacing(4.0F);
    bottom.configure_all_styles([](Style& style) {
        style.background_color(rgba(33, 34, 38, 240));
        style.border(BorderEdge{1.0F, BorderType::Solid});
        style.border_color(rgba(149, 204, 221, 60));
        style.border_radius(4.0F);
        style.padding({8.0F, 6.0F});
    });

    auto& tab_bar = bottom.add<Container>("editor-tab-bar", StackDirection::Horizontal);
    tab_bar.set_size({grow(), fit()});
    tab_bar.set_spacing(2.0F);
    tab_bar.set_input_mode(InputMode::None);

    m_library_tab = &tab_bar.add<EditorTabButton>("library");

    m_tab_body = &bottom.add<Container>("editor-tab-body");
    m_tab_body->set_size({grow(), grow()});
    m_tab_body->set_scrollable(true);
    m_tab_body->set_spacing(6.0F);
    m_tab_body->configure_all_styles([](Style& style) { style.padding({8.0F, 0.0F}); });
}

void EditorPanel::on_update(float) {
    if (!m_initialized) {
        m_initialized = true;
        m_library_tab->set_selected(true);
        build_library(*m_tab_body);
    }

    LevelEditor* editor = m_game.editor();
    if (editor != m_editor) {
        m_editor = editor;
        if (editor != nullptr) {
            set_mode(editor->mode());
        }
    }

    const ui::Rect rect = m_game_space->layout().visual_rect();
    if (rect.valid()) {
        if (auto* viewport = m_game.get_viewport(0)) {
            const ImVec2 size = rect.size();
            viewport->set_position({rect.min.x, rect.min.y});
            viewport->set_size({size.x, size.y});
        }
    }
}

void EditorPanel::key_press_event(UiEvent& event) {
    if (m_on_key_press) m_on_key_press(event);
}

void EditorPanel::set_mode(EditorMode mode) {
    if (auto* editor = m_game.editor()) {
        editor->set_mode(mode);
    }

    m_move_button->set_selected(mode == EditorMode::Move);
    m_select_button->set_selected(mode == EditorMode::Select);
}

void EditorPanel::build_library(Container& body) {
    body.add<TextWidget>("Object library").set_font(surface().get_primary_font(), 32.0F);
    body.add<TextWidget>("not implemented yet");
}
