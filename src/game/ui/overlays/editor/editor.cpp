#include "editor.hpp"
#include "game/dash.hpp"
#include "game/editor/editor.hpp"
#include "core/components/rigid-body.hpp"
#include "controls.hpp"
#include "properties-panel.hpp"

#include <array>
#include <utility>
#include <raymath.h>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/layout/container.hpp>

using namespace ui;

static std::array<Vector2, 4> selection_corners(const RigidBody& body, const Viewport& viewport) {
    const Rectangle bounds = body.get_bounding_box();
    const Vector2 center = {bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
    std::array<Vector2, 4> corners = {
        {{bounds.x, bounds.y},
         {bounds.x + bounds.width, bounds.y},
         {bounds.x + bounds.width, bounds.y + bounds.height},
         {bounds.x, bounds.y + bounds.height}}
    };
    for (auto& corner : corners) {
        corner = Vector2Add(center, Vector2Rotate(Vector2Subtract(corner, center), body.get_rotation() * DEG2RAD));
        corner = GetWorldToScreen2D(corner, viewport.camera().view());
    }
    return corners;
}

class EditorPanel : public LayerContainer {
public:
    EditorPanel(Dash& game, Surface& ui_surface, InputCallback on_key_press, std::function<void()> on_exit);
    bool has_unsaved_changes() const;
    void reset();

protected:
    void on_update(float frametime) override;

private:
    void event(UiEvent& event) override;
    void key_press_event(UiEvent& event) override;
    void set_mode(EditorMode mode);
    void build_library(Container& body);
    void draw_selection(const LevelEditor& editor, const Viewport& viewport);

    Dash& m_game;
    InputCallback m_on_key_press;
    Container* m_game_space = nullptr;
    Container* m_tab_body = nullptr;
    EditorTabButton* m_library_tab = nullptr;
    EditorModeButton* m_move_button = nullptr;
    EditorModeButton* m_select_button = nullptr;
    PropertiesPanel* m_properties = nullptr;
    DashButton* m_update_button = nullptr;
    LevelEditor* m_editor = nullptr;
    bool m_initialized = false;
};

EditorPanel::EditorPanel(Dash& game, Surface& ui_surface, InputCallback on_key_press, std::function<void()> on_exit)
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
    actions.set_spacing(6.0F);
    actions.set_anchor(Anchor::TopRight);
    m_update_button = &actions.add<DashButton>("update");
    m_update_button->set_visible(false);
    m_update_button->on_click([this] {
        if (m_properties->save()) {
            m_update_button->set_visible(false);
        }
    });

    auto* close_icon = ui_surface.runtime().textures().add("editor-close", RESOURCES_LOCATION / "ui/close.svg");
    actions.add<EditorIconButton>(close_icon).on_click(std::move(on_exit));

    auto& workspace = add<Container>("editor-workspace", StackDirection::Horizontal);
    workspace.set_size({grow(), percent(60.0F)});
    workspace.set_spacing(6.0F);
    workspace.set_input_mode(InputMode::None);

    m_properties = &workspace.add<PropertiesPanel>(game);

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
    m_update_button->set_visible(editor != nullptr && has_unsaved_changes());
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
            if (editor != nullptr && parent()->input_enabled()) {
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
    m_editor = nullptr;
    m_properties->reset();
    m_update_button->set_visible(false);
}

void EditorPanel::draw_selection(const LevelEditor& editor, const Viewport& viewport) {
    const ImU32 color = ImColor(149, 204, 221, 255);

    const Vector2 origin = viewport.position();
    const Vector2 size = viewport.size();

    auto* drawlist = ImGui::GetForegroundDrawList();
    drawlist->PushClipRect({origin.x, origin.y}, {origin.x + size.x, origin.y + size.y}, true);

    for (const auto* entity : editor.selected_objects()) {
        const auto* body = entity->get_component<RigidBody>();
        if (body == nullptr) {
            continue;
        }

        const auto corners = selection_corners(*body, viewport);
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

void EditorPanel::key_press_event(UiEvent& event) {
    if (m_on_key_press) m_on_key_press(event);
}

void EditorPanel::event(UiEvent& event) {
    // ignora native text fields (imgui) and other shortcuts
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
    }

    m_move_button->set_selected(mode == EditorMode::Move);
    m_select_button->set_selected(mode == EditorMode::Select);
}

void EditorPanel::build_library(Container& body) {
    body.add<TextWidget>("not implemented yet");
}

EditorOverlay::EditorOverlay(Dash& game, Surface& ui_surface, InputCallback on_key_press, std::function<void()> on_exit)
    : LayerContainer("editor") {
    set_size({grow(), grow()});
    set_input_mode(InputMode::None);
    set_input_enabled(false);
    set_visible(false);

    m_panel = &add<EditorPanel>(game, ui_surface, std::move(on_key_press), std::move(on_exit));
}

bool EditorOverlay::has_unsaved_changes() const {
    return m_panel->has_unsaved_changes();
}

void EditorOverlay::reset() {
    m_panel->reset();
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
