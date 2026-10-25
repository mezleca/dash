#include "gameplay-hud.hpp"

#include "game/dash.hpp"

#include <imgui-ui/imgui/painter.hpp>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/widgets/widget.hpp>
#include <algorithm>

using namespace ui;

class GameplayProgress : public Widget {
public:
    explicit GameplayProgress(Dash& game) : Widget("gameplay-progress", "GameplayProgress", InputMode::None), m_game(game) {
        set_size({percent(35), px(4)});
        set_anchor(Anchor::TopCenter);
    }

private:
    void paint_content(const PaintContext& context) override {
        const auto* level = m_game.current_level();
        if (level == nullptr) return;

        const Rect rect = context.content_rect;
        const float progress = std::clamp(level->progress() / 100.0F, 0.0F, 1.0F);
        context.painter.rect_filled(rect, rgba(255, 255, 255, 40));
        context.painter.rect_filled({rect.min, {rect.min.x + rect.size().x * progress, rect.max.y}}, rgb(208, 231, 230));
    }

    Dash& m_game;
};

GameplayHud::GameplayHud(Dash& game) : LayerContainer("gameplay") {
    set_size({grow(), grow()});
    set_input_mode(InputMode::None);
    set_input_enabled(false);
    set_visible(false);

    auto& info = add<Container>("gameplay-info");
    info.set_size({fit(), fit()});
    info.set_input_mode(InputMode::None);
    info.add<TextWidget>("dash " + std::string(DASH_VERSION));
    m_fps = &info.add<TextWidget>("FPS: 0");
    add<GameplayProgress>(game);
}

void GameplayHud::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 18);
    configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0, BorderType::None});
        style.padding({12, 12});
    });
}

void GameplayHud::on_update(float) {
    m_fps->set_text("FPS: " + std::to_string(GetFPS()));
}
