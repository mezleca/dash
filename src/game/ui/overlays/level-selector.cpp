#include "level-selector.hpp"
#include "../widgets/level-card.hpp"
#include "../widgets/carousel-container.hpp"
#include "../../game.hpp"

#include <imgui-ui/widgets/text.hpp>

using namespace ui;

LevelSelectorLayer::LevelSelectorLayer(std::string id, std::function<void(DashLevel&)> on_select)
    : MenuOptionLayer("level-selector-" + id) {
    set_size({grow(), grow()});
    configure_all_styles([](Style& style) {
        style.border(BORDER_NONE);
        style.background_color(
            gradient(GradientType::Linear, {{0.0F, rgb(33, 33, 33)}, {1.0F, rgb(16, 16, 16)}}, {0, 0}, {0, 1})
        );
    });

    if (game.levels().empty()) {
        add<TextWidget>("no levels found");
        return;
    }

    auto& carousel = add<CarouselContainer>("levels");
    for (const auto& level : game.levels()) {
        auto& card = carousel.add<LevelCard>(*level);

        if (on_select) {
            card.on_click([on_select, level = level.get()] { on_select(*level); });
        }
    }
}
