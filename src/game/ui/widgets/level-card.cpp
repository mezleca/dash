#include "level-card.hpp"
#include "game/level/level.hpp"

#include <imgui-ui/surface.hpp>

using namespace ui;

LevelCard::LevelCard(const DashLevel& level) : ButtonWidget(level.name(), {percent(50), percent(50)}) {}

void LevelCard::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 48);

    configure_all_styles([](Style& style) {
        style.background_color(rgb(36, 36, 36));
        style.padding({16.0F, 16.0F});
        style.border_radius(4.0F);
        style.border_thickness(2.0F);
    });
}
