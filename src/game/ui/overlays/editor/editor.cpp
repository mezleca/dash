#include "editor.hpp"
#include "game/dash.hpp"
#include "game/editor/editor.hpp"
#include "game/ui/theme.hpp"
#include "core/components/rigid-body.hpp"
#include "game/objects/world/invisible-sprite.hpp"
#include "controls.hpp"
#include "properties-panel.hpp"
#include "library-panel.hpp"
#include "behaviour-dialog.hpp"

#include <utility>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <imgui-ui/diagnostics/debugger.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/layout/container.hpp>

using namespace ui;

class EditorPanel : public LayerContainer {
public:
    EditorPanel(
        Dash& game, Surface& ui_surface, InputCallback on_key_press, std::function<void()> on_exit,
        std::function<void(Entity&, Behaviour*)> open_behaviour_dialog
    );
    bool has_unsaved_changes() const;
    void reset();
    void behaviours_changed();

protected:
    void apply_theme_defaults(const Theme& theme) override;
    void on_update(float frametime) override;

private:
    void event(UiEvent& event) override;
    void key_press_event(UiEvent& event) override;
    void set_mode(EditorMode mode);
    void draw_selection(const LevelEditor& editor, const Viewport& viewport);
    void draw_label(const Entity& object, const LevelEditor& editor, const Viewport& viewport);

    Dash& m_game;
    InputCallback m_on_key_press;

    Container* m_game_space = nullptr;
    Container* m_tools = nullptr;
    Container* m_bottom = nullptr;

    EditorModeButton* m_move_button = nullptr;
    EditorModeButton* m_select_button = nullptr;
    PropertiesPanel* m_properties = nullptr;
    DashButton* m_update_button = nullptr;
};

EditorPanel::EditorPanel(
    Dash& game, Surface& ui_surface, InputCallback on_key_press, std::function<void()> on_exit,
    std::function<void(Entity&, Behaviour*)> open_behaviour_dialog
)
    : LayerContainer("editor-panel"), m_game(game), m_on_key_press(std::move(on_key_press)) {
    set_size({grow(), grow()});
    set_direction(StackDirection::Vertical);
    set_spacing(4.0F);
    set_input_mode(InputMode::None);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({6.0F, 6.0F});
    });

    auto& mode_row = add<Container>("editor-mode-row", StackDirection::Horizontal);
    m_tools = &mode_row;
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

    auto* pointer_icon = ui_surface.runtime().textures().add("editor-pointer", RESOURCES_LOCATION / "ui/editor/pointer.svg");
    auto* hand_icon = ui_surface.runtime().textures().add("editor-hand", RESOURCES_LOCATION / "ui/editor/hand.svg");

    m_select_button = &mode_panel.add<EditorModeButton>(pointer_icon, true);
    m_select_button->on_click([this] { set_mode(EditorMode::Select); });
    m_select_button->set_tooltip("also works by pressing s");

    m_move_button = &mode_panel.add<EditorModeButton>(hand_icon, false);
    m_move_button->on_click([this] { set_mode(EditorMode::Move); });
    m_move_button->set_tooltip("also works by pressing m");

    auto& actions = mode_row.add<Container>("editor-actions", StackDirection::Horizontal);
    actions.set_size({fit(), fit()});
    actions.set_spacing(4.0F);
    actions.set_anchor(Anchor::TopRight);
    m_update_button = &actions.add<DashButton>("update");
    m_update_button->set_visible(false);
    m_update_button->on_click([this] {
        if (m_properties->save()) {
            m_update_button->set_visible(false);
        }
    });

    actions.add<DashCloseButton>().on_click(std::move(on_exit));

    auto& workspace = add<Container>("editor-workspace", StackDirection::Horizontal);
    workspace.set_size({grow(), percent(65.0F)});
    workspace.set_spacing(4.0F);
    workspace.set_input_mode(InputMode::None);

    m_properties = &workspace.add<PropertiesPanel>(game, std::move(open_behaviour_dialog));

    m_game_space = &workspace.add<Container>("editor-game-space");
    m_game_space->set_size({grow(), grow()});
    m_game_space->set_input_mode(InputMode::None);
    m_game_space->configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({0.0F, 0.0F});
    });

    auto& bottom = add<Container>("editor-bottom");
    m_bottom = &bottom;
    bottom.set_size({grow(), grow()});
    bottom.set_spacing(4.0F);

    auto& tab_bar = bottom.add<Container>("editor-tab-bar", StackDirection::Horizontal);
    tab_bar.set_size({grow(), fit()});
    tab_bar.configure_all_styles([](Style& style) {
        style.border_radius(0.0F);
        style.border_bottom(1.0F);
        style.border_color(rgba(149, 204, 221, 60));
    });
    tab_bar.set_spacing(12.0F);
    tab_bar.set_input_mode(InputMode::None);

    tab_bar.add<EditorTabButton>("library", true);

    auto& tab_body = bottom.add<Container>("editor-tab-body");
    tab_body.set_size({grow(), grow()});
    tab_body.set_spacing(4.0F);
    tab_body.configure_all_styles([](Style& style) { style.padding({12.0F, 4.0F}); });
    tab_body.add<LibraryPanel>(game);
}

void EditorPanel::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);
    m_bottom->configure_all_styles([&theme](Style& style) { dash_ui::editor_panel_style(style, theme); });
}

void EditorPanel::on_update(float) {
    LevelEditor* editor = m_game.editor();
    const bool picking = editor != nullptr && editor->mode() == EditorMode::PickTarget;

    m_tools->set_input_enabled(!picking);
    m_bottom->set_input_enabled(!picking);
    m_properties->set_input_enabled(!picking);

    const bool keyboard_available = editor != nullptr && editor->mode() != EditorMode::PickTarget && input_enabled() &&
                                    parent()->input_enabled() && !surface().debugger()->is_open() &&
                                    !ImGui::GetIO().WantTextInput;

    if (keyboard_available && IsKeyPressed(KEY_DELETE)) {
        editor->handle_event({EditorEventType::Delete});
    }

    if (keyboard_available && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        if (IsKeyPressed(KEY_Z)) {
            editor->undo();
            m_properties->refresh();
        } else if (IsKeyPressed(KEY_C)) {
            editor->copy_selection();
        } else if (IsKeyPressed(KEY_V)) {
            editor->paste();
        }
    }

    m_update_button->set_visible(editor != nullptr && has_unsaved_changes());
    if (editor != nullptr) {
        m_move_button->set_selected(editor->mode() == EditorMode::Move);
        m_select_button->set_selected(editor->mode() == EditorMode::Select);
    }

    const ui::Rect rect = m_game_space->layout().visual_rect();

    if (rect.valid()) {
        if (auto* viewport = m_game.get_viewport(0)) {
            const ImVec2 size = rect.size();
            viewport->set_position({rect.min.x, rect.min.y});
            viewport->set_size({size.x, size.y});
            if (editor != nullptr && input_enabled() && parent()->input_enabled()) {
                draw_selection(*editor, *viewport);
            }
        }
    }
}

bool EditorPanel::has_unsaved_changes() const {
    const auto* editor = m_game.editor();
    return (editor != nullptr && editor->has_unsaved_changes()) || m_properties->has_draft_changes();
}

void EditorPanel::reset() {
    m_properties->reset();
    m_update_button->set_visible(false);
}

void EditorPanel::behaviours_changed() {
    if (auto* editor = m_game.editor()) editor->mark_changed();
    m_properties->refresh_behaviours();
}

void EditorPanel::draw_selection(const LevelEditor& editor, const Viewport& viewport) {
    const ImU32 color = ImColor(149, 204, 221, 255);

    const Vector2 origin = viewport.position();
    const Vector2 size = viewport.size();

    auto* drawlist = ImGui::GetForegroundDrawList();
    drawlist->PushClipRect({origin.x, origin.y}, {origin.x + size.x, origin.y + size.y}, true);

    auto* font = surface().get_primary_font();
    ImGui::PushFont(font != nullptr ? font : ImGui::GetFont(), 18.0F);
    for (const auto* object : m_game.current_level()->objects())
        draw_label(*object, editor, viewport);
    if (const auto* preview = editor.preview_object()) draw_label(*preview, editor, viewport);
    ImGui::PopFont();

    for (const auto* entity : editor.selected_objects()) {
        const auto* body = entity->get_component<RigidBody>();
        if (body == nullptr) {
            continue;
        }

        const auto corners = editor.selection_corners(*entity);
        ImVec2 points[4];
        for (std::size_t index = 0; index < corners.size(); ++index) {
            points[index] = {origin.x + corners[index].x, origin.y + corners[index].y};
        }

        drawlist->AddPolyline(points, 4, color, ImDrawFlags_Closed, 2.0F);

        if (editor.selected_objects().size() == 1) {
            for (const auto& point : points) {
                const float radius = LevelEditor::JOINT_RADIUS;
                const ImVec2 minimum = {point.x - radius, point.y - radius};
                const ImVec2 maximum = {point.x + radius, point.y + radius};
                drawlist->AddRectFilled(minimum, maximum, ImColor(26, 27, 31, 255));
                drawlist->AddRect(minimum, maximum, color, 0.0F, 0, 2.0F);
            }
        }
    }
    if (const auto rectangle = editor.selection_rectangle()) {
        const ImVec2 minimum = {origin.x + rectangle->x, origin.y + rectangle->y};
        const ImVec2 maximum = {minimum.x + rectangle->width, minimum.y + rectangle->height};
        drawlist->AddRectFilled(minimum, maximum, ImColor(149, 204, 221, 35));
        drawlist->AddRect(minimum, maximum, color);
    }

    drawlist->PopClipRect();
}

void EditorPanel::draw_label(const Entity& object, const LevelEditor& editor, const Viewport& viewport) {
    const auto* sprite = object.get_component<InvisibleSprite>();
    if (sprite == nullptr || !object.is_visible()) return;

    const auto corners = editor.selection_corners(object);
    const Vector2 origin = viewport.position();
    const ImVec2 size = ImGui::CalcTextSize(sprite->label().c_str());
    const ImVec2 position = {
        origin.x + (corners[0].x + corners[2].x - size.x) * 0.5F, origin.y + (corners[0].y + corners[2].y - size.y) * 0.5F
    };
    ImGui::GetForegroundDrawList()->AddText(position, IM_COL32_WHITE, sprite->label().c_str());
}

void EditorPanel::key_press_event(UiEvent& event) {
    if (m_on_key_press) m_on_key_press(event);
}

void EditorPanel::event(UiEvent& event) {
    if (event.propagation_stopped || event.type != EventType::TextInput || ImGui::GetIO().WantTextInput ||
        IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT) ||
        IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) {
        return;
    }

    if (event.text == "m" || event.text == "M") {
        set_mode(EditorMode::Move);
    } else if (event.text == "s" || event.text == "S") {
        set_mode(EditorMode::Select);
    } else {
        return;
    }

    event.stop_propagation();
    event.block_native_input();
}

void EditorPanel::set_mode(EditorMode mode) {
    if (auto* editor = m_game.editor()) {
        editor->set_mode(mode);
        mode = editor->mode();
    }

    m_move_button->set_selected(mode == EditorMode::Move);
    m_select_button->set_selected(mode == EditorMode::Select);
}

EditorOverlay::EditorOverlay(Dash& game, Surface& ui_surface, InputCallback on_key_press, std::function<void()> on_exit)
    : LayerContainer("editor"), m_game(game) {
    set_size({grow(), grow()});
    set_input_mode(InputMode::None);
    set_input_enabled(false);
    set_visible(false);

    m_panel = &add<EditorPanel>(
        game, ui_surface, std::move(on_key_press), std::move(on_exit), [this, &game](Entity& object, Behaviour* behaviour) {
            if (auto* editor = game.editor()) editor->handle_event({EditorEventType::FinishInteraction});
            m_behaviour_dialog->open(object, behaviour);
        }
    );
    m_behaviour_dialog = &add<BehaviourDialog>(game, [this] { m_panel->behaviours_changed(); });
    m_behaviour_dialog->on_state_change([this](ModalState state) {
        m_panel->set_input_enabled(state != ModalState::Open);
        if (state == ModalState::Closed) surface().input_router().set_focus(m_panel);
    });
}

bool EditorOverlay::has_unsaved_changes() const {
    return m_panel->has_unsaved_changes();
}

void EditorOverlay::reset() {
    m_behaviour_dialog->close();
    m_panel->reset();
}

bool EditorOverlay::close_behaviour_dialog() {
    if (!m_behaviour_dialog->visible()) {
        return false;
    }

    if (m_behaviour_dialog->state() == ModalState::Minimized) {
        m_behaviour_dialog->cancel_target_selection();
        return true;
    }

    m_behaviour_dialog->close();

    return true;
}

bool EditorOverlay::behaviour_dialog_open() const {
    return m_behaviour_dialog->state() == ModalState::Open;
}

bool EditorOverlay::input_available() const {
    return m_game.editor() != nullptr && m_game.level_state() == LevelState::EDITING && !surface().debugger()->is_open() &&
           IsWindowFocused();
}

void EditorOverlay::process_pointer_event(UiEvent& event, bool pointer_over_ui) {
    if (!input_available()) {
        return;
    }

    auto* viewport = m_game.get_viewport(0);

    if (viewport == nullptr) {
        return;
    }

    EditorEvent input = {EditorEventType::PointerMove};
    const Vector2 origin = viewport->position();

    input.m_position = {event.position.x - origin.x, event.position.y - origin.y};
    input.m_snap = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    switch (event.type) {
        case EventType::PointerDown:
        case EventType::PointerUp:
            if (event.button != PointerButton::Left && event.button != PointerButton::Right) {
                return;
            }

            input.m_button = event.button == PointerButton::Right ? MOUSE_BUTTON_RIGHT : MOUSE_BUTTON_LEFT;
            input.m_type = event.type == EventType::PointerDown ? EditorEventType::PointerDown : EditorEventType::PointerUp;
            break;
        case EventType::PointerMove:
            break;
        case EventType::Scroll:
            input.m_type = EditorEventType::Scroll;
            input.m_scroll = event.scroll.y;
            break;
        default:
            return;
    }

    // allow releases over panels, but start interactions inside the viewport.
    if ((input.m_type == EditorEventType::PointerDown || input.m_type == EditorEventType::Scroll) &&
        (pointer_over_ui || !viewport->contains_mouse())) {
        return;
    }

    m_game.editor()->handle_event(input);
}

void EditorOverlay::update_input(bool active) {
    auto* editor = m_game.editor();

    if (editor == nullptr || m_game.level_state() != LevelState::EDITING) {
        return;
    }

    // commit native edits after release or focus loss.
    if (!IsWindowFocused() || (!IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !ImGui::IsAnyItemActive())) {
        editor->commit_changes();
    }

    if (!active || !input_available()) {
        editor->handle_event({EditorEventType::FinishInteraction});
        return;
    }

    if (auto* viewport = m_game.get_viewport(0)) {
        const bool snap = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        // refresh modifiers and recover intercepted releases.
        const int button = editor->pointer_button();
        const auto type = IsMouseButtonDown(button) ? EditorEventType::PointerMove : EditorEventType::PointerUp;

        editor->handle_event({type, viewport->mouse_position(), 0.0F, snap, button});
    }
}

void EditorOverlay::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({6.0F, 6.0F});
    });

    set_font(surface().get_primary_font(), 26);
}
