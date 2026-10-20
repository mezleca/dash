#include "editor.hpp"

#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

EditorOverlay::EditorOverlay(Dash& game, ui::InputCallback on_key_press) : LayerContainer("editor") {
    set_size({grow(), grow()});
    set_input_mode(InputMode::None);
    set_input_enabled(false);
    set_visible(false);

    add<EditorPanel>(game, std::move(on_key_press));
}

void EditorOverlay::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);

    configure_all_styles([theme](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({6.0F, 6.0F});
    });

    set_font(surface().get_primary_font(), 26);
}
