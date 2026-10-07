#include "editor.hpp"
#include "../../game.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/widgets/image.hpp>

using namespace ui;

DebugInfo::DebugInfo() : LayerContainer("editor-debug-info") {
    set_size({fit(), fit()});
    set_anchor(Anchor::TopLeft);
    set_direction(StackDirection::Vertical);
    set_spacing(4.0F);
    set_input_mode(InputMode::Target);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BORDER_NONE);
        style.padding({8.0F, 8.0F});
    });

    style(StyleType::HOVER).background_color(rgba(16, 16, 16, 180));

    m_camera_position = &add<TextWidget>("Camera (world):");
    m_object_count = &add<TextWidget>("Objects:");
    m_camera_zoom = &add<TextWidget>("Zoom:");
    m_camera_rotation = &add<TextWidget>("Rotation:");
    m_frame_stats = &add<TextWidget>("FPS:");
}

void DebugInfo::on_update(float frametime) {
    const auto& camera = game.camera().transform();
    const DashLevel* level = game.current_level();
    const std::size_t object_count = level != nullptr ? level->objects().size() : 0;

    m_camera_position->set_text(TextFormat("Camera (world): %.1f, %.1f", camera.target.x, camera.target.y));
    m_object_count->set_text(TextFormat("Objects: %zu", object_count));
    m_camera_zoom->set_text(TextFormat("Zoom: %.2f", camera.zoom));
    m_camera_rotation->set_text(TextFormat("Rotation: %.1f", camera.rotation));
    m_frame_stats->set_text(TextFormat("FPS: %d | Frame: %.2f ms", GetFPS(), frametime * 1000.0F));
}

EditorPanel::EditorPanel() : LayerContainer("editor-panel") {
    set_size({fit(), fit()});
    set_anchor(Anchor::BottomRight);

    set_input_mode(InputMode::Blocker);
    configure_all_styles([](Style& style) {
        style.background_color(rgba(16, 16, 16, 180));
        style.border(BORDER_NONE);
    });

    auto* edit_texture = game.surface().runtime().textures().add("edit", RESOURCES_LOCATION / "ui/edit.svg");
    m_edit_icon = &add<ImageWidget>(edit_texture);
    m_edit_icon->configure_all_styles([](Style& style) { style.color(rgb(255, 255, 255)); });
    m_edit_icon->on_event([this](UiEvent& event) {
        if (event.type != EventType::Click || event.button != PointerButton::Left) return;

        set_compact(false);
        event.stop_propagation();
        event.block_native_input();
    });

    m_content = &add<Container>("editor-panel-content");
    m_content->set_size({fit(), fit()});
    m_content->set_spacing(10.0F);

    auto& header = m_content->add<Container>("editor-panel-header");
    header.set_size({grow(), fit()});
    header.set_content_alignment(Anchor::TopRight);

    auto* close_texture = game.surface().runtime().textures().add("close", RESOURCES_LOCATION / "ui/close.svg");
    auto& close = header.add<ImageWidget>(close_texture);
    close.configure_all_styles([](Style& style) { style.color(rgb(255, 255, 255)); });
    close.style(StyleType::HOVER).background_color(rgba(255, 255, 255, 30)).border_radius(4.0F);
    close.on_event([this](UiEvent& event) {
        if (event.type != EventType::Click || event.button != PointerButton::Left) return;

        set_compact(true);
        event.stop_propagation();
        event.block_native_input();
    });

    m_content->add<TextWidget>("Hello, World!");
    set_compact(true);
}

void EditorPanel::set_compact(bool compact) {
    m_edit_icon->set_visible(compact);
    m_content->set_visible(!compact);
    set_size({fit(), compact ? fit() : grow()});

    const float padding = compact ? 0.0F : 10.0F;
    configure_all_styles([compact, padding](Style& style) {
        style.padding({padding, padding});
        style.background_color(rgba(20, 24, 32, compact ? 180 : 240));
        style.border(compact ? BORDER_NONE : BORDER_ALL);
        style.border_color(rgba(140, 160, 190, 140));
        style.border_thickness(1.0F);
        style.border_radius(compact ? 0.0F : 8.0F);
    });
}

EditorLayer::EditorLayer() : LayerContainer("editor") {
    set_size({grow(), grow()});
    set_input_mode(InputMode::None);
    set_enabled(false);
    set_visible(false);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BORDER_NONE);
        style.padding({0.0F, 0.0F});
    });

    add<DebugInfo>();
    add<EditorPanel>();
}

void EditorLayer::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(20));
}
