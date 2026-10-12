#include "editor.hpp"
#include "game/dash.hpp"

#include <imgui-ui/widgets/text.hpp>

using namespace ui;

DebugPanel::DebugPanel(Dash& game) : LayerContainer("editor-debug-panel"), m_game(game) {
    set_size({fit(), fit()});
    set_direction(StackDirection::Vertical);
    set_spacing(4.0F);
    set_input_mode(InputMode::Target);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({8.0F, 8.0F});
    });

    style(StyleType::HOVER).background_color(rgba(16, 16, 16, 180));

    m_camera_position = &add<TextWidget>("Camera (world):");
    m_object_count = &add<TextWidget>("Objects:");
    m_camera_zoom = &add<TextWidget>("Zoom:");
    m_camera_rotation = &add<TextWidget>("Rotation:");
    m_frame_stats = &add<TextWidget>("FPS:");
}

void DebugPanel::on_update(float frametime) {
    const auto& camera = m_game.camera().view();
    const DashLevel* level = m_game.current_level();
    const std::size_t object_count = level != nullptr ? level->objects().size() : 0;

    m_camera_position->set_text(TextFormat("Camera (world): %.1f, %.1f", camera.target.x, camera.target.y));
    m_object_count->set_text(TextFormat("Objects: %zu", object_count));
    m_camera_zoom->set_text(TextFormat("Zoom: %.2f", camera.zoom));
    m_camera_rotation->set_text(TextFormat("Rotation: %.1f", camera.rotation));
    m_frame_stats->set_text(TextFormat("FPS: %d | Frame: %.2f ms", GetFPS(), frametime * 1000.0F));
}
