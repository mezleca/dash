#include "level-selector.hpp"
#include "../widgets/level-card.hpp"
#include "../widgets/carousel-container.hpp"
#include "game/dash.hpp"

#include <imgui-ui/widgets/text.hpp>

using namespace ui;

LevelSelectorLayer::LevelSelectorLayer(Dash& game, std::string id, std::function<void(DashLevel&)> on_select)
    : MenuOptionLayer("level-selector-" + id) {
    set_size({grow(), grow()});
    configure_all_styles([](Style& style) {
        style.border(BORDER_NONE);
        style.background_color(
            gradient(GradientType::Linear, {{0.0F, rgb(33, 33, 33)}, {1.0F, rgb(16, 16, 16)}}, {0, 0}, {0, 1})
        );
    });

    auto& levels = game.levels();
    if (levels.empty()) {
        add<TextWidget>("no levels found");
        return;
    }

    auto& carousel = add<CarouselContainer>("levels");
    for (auto& entry : levels) {
        DashLevel& level = *entry.second;
        auto& card = carousel.add<LevelCard>(level);

        if (on_select) {
            card.on_click([on_select, selected = &level] { on_select(*selected); });
        }
    }
}
