#include "game-ui.hpp"
#include "overlays/exit.hpp"
#include "overlays/level-selector.hpp"
#include "overlays/option-layer.hpp"
#include "overlays/settings.hpp"
#include "widgets/menu-button.hpp"
#include "../game.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>

#include <array>
#include <utility>

using namespace ui;

GameUI::GameUI() : LayerContainer("game-ui") {
    set_size({grow(), grow()});

    build_menu();
    build_option_panels();
    build_gameplay_panels();
    build_loading();

    // blocking panels receive keyboard input before the root of the ui tree.
    for (MenuOptionLayer* layer : {m_levels, m_editor, m_settings, m_exit, m_pause, m_death}) {
        layer->on_key_press([this](UiEvent& event) { this->event(event); });
    }
}

void GameUI::build_menu() {
    auto& menu = add<Container>("main-menu");
    m_menu = &menu;
    menu.set_size({grow(), grow()});
    menu.set_content_alignment(Anchor::Center);
    menu.add<TextWidget>("DASH").set_font(game.surface().get_primary_font(56));

    auto& actions = menu.add<Container>("menu-actions", StackDirection::Horizontal);
    actions.set_size({grow(), fit()});
    actions.set_content_alignment(Anchor::Center);
    actions.set_spacing(10.0F);
    actions.configure_all_styles([](Style& style) { style.padding({12.0F, 12.0F}); });

    const std::array<std::pair<const char*, GameScreen>, 4> options = {{
        {"play", GameScreen::Levels},
        {"editor", GameScreen::Editor},
        {"settings", GameScreen::Settings},
        {"exit", GameScreen::Exit},
    }};

    for (const auto& [text, screen] : options) {
        actions.add<MenuButton>(text).on_click([this, screen] { show(screen); });
    }
}

void GameUI::build_option_panels() {
    m_levels = &add<LevelSelectorLayer>("play", [this](DashLevel& level) { play_level(level); });
    m_editor = &add<LevelSelectorLayer>("editor");
    m_settings = &add<SettingsLayer>("game");
    m_exit = &add<ExitLayer>("game", [this] { close_panel(); });
}

void GameUI::build_gameplay_panels() {
    auto& pause = add<MenuOptionLayer>("pause");
    m_pause = &pause;
    pause.set_content_alignment(Anchor::Center);
    auto& pause_actions = pause.add<Container>("pause-actions");
    pause_actions.set_size({fit(), fit()});
    pause_actions.set_content_alignment(Anchor::Center);
    pause_actions.set_spacing(12.0F);
    pause_actions.add<TextWidget>("paused");
    pause_actions.add<MenuButton>("resume").on_click([] { game.resume_level(); });
    pause_actions.add<MenuButton>("settings").on_click([this] { show(GameScreen::Settings); });
    pause_actions.add<MenuButton>("main menu").on_click([] { game.return_to_menu(); });

    auto& death = add<MenuOptionLayer>("death");
    m_death = &death;
    death.set_content_alignment(Anchor::Center);
    auto& death_actions = death.add<Container>("death-actions");
    death_actions.set_size({fit(), fit()});
    death_actions.set_content_alignment(Anchor::Center);
    death_actions.set_spacing(12.0F);
    death_actions.add<TextWidget>("you died");
    death_actions.add<MenuButton>("retry").on_click([] { game.restart_current_level(); });
    death_actions.add<MenuButton>("main menu").on_click([] { game.return_to_menu(); });
}

void GameUI::build_loading() {
    // the loading label stays above the selector without taking input before the fade starts.
    m_loading = &add<LayerContainer>("level-loading");
    m_loading->set_size({grow(), grow()});
    m_loading->set_content_alignment(Anchor::BottomCenter);
    m_loading->set_input_mode(InputMode::None);
    m_loading->set_enabled(false);
    m_loading->set_visible(false);
    m_loading->add<TextWidget>("loading...").set_font(game.surface().get_primary_font(32));
}
