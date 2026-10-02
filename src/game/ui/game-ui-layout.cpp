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

    // blocking panels receive keyboard input before the root of the ui tree.
    for (const auto& entry : m_screens) {
        if (auto* layer = dynamic_cast<MenuOptionLayer*>(entry.second)) {
            layer->on_key_press([this](UiEvent& event) { this->event(event); });
        }
    }
}

void GameUI::build_menu() {
    auto& menu = add<Container>("main-menu");
    m_screens.emplace(GameScreen::Menu, &menu);
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
        actions.add<MenuButton>(text).on_click([this, screen] { show_screen(screen); });
    }
    menu.set_visible(true);
    m_open.push_back(GameScreen::Menu);
}

void GameUI::build_option_panels() {
    m_screens.emplace(GameScreen::Levels, &add<LevelSelectorLayer>("play", [this](DashLevel& level) { play_level(level); }));
    m_screens.emplace(GameScreen::Editor, &add<LevelSelectorLayer>("editor"));
    m_screens.emplace(GameScreen::Settings, &add<SettingsLayer>("game"));
    m_screens.emplace(GameScreen::Exit, &add<ExitLayer>("game", [this] { hide_screen(GameScreen::Exit); }));
    auto& loading = add<LayerContainer>("level-loading");
    m_screens.emplace(GameScreen::Loading, &loading);
    loading.set_size({grow(), grow()});
    loading.set_content_alignment(Anchor::BottomCenter);
    loading.set_input_mode(InputMode::Blocker);
    loading.set_enabled(false);
    loading.set_visible(false);
    loading.add<TextWidget>("loading...").set_font(game.surface().get_primary_font(32));
}

void GameUI::build_gameplay_panels() {
    auto& gameplay = add<LayerContainer>("gameplay");
    m_screens.emplace(GameScreen::Gameplay, &gameplay);
    gameplay.set_size({grow(), grow()});
    gameplay.set_input_mode(InputMode::None);
    gameplay.set_enabled(false);
    gameplay.set_visible(false);

    auto& pause = add<MenuOptionLayer>("pause");
    m_screens.emplace(GameScreen::Pause, &pause);
    pause.set_content_alignment(Anchor::Center);
    auto& pause_actions = pause.add<Container>("pause-actions");
    pause_actions.set_size({fit(), fit()});
    pause_actions.set_content_alignment(Anchor::Center);
    pause_actions.set_spacing(12.0F);
    pause_actions.add<TextWidget>("paused");
    pause_actions.add<MenuButton>("resume").on_click([] { game.resume_level(); });
    pause_actions.add<MenuButton>("settings").on_click([this] { show_screen(GameScreen::Settings); });
    pause_actions.add<MenuButton>("main menu").on_click([] { game.return_to_menu(); });

    auto& death = add<MenuOptionLayer>("death");
    m_screens.emplace(GameScreen::Death, &death);
    death.set_content_alignment(Anchor::Center);
    auto& death_actions = death.add<Container>("death-actions");
    death_actions.set_size({fit(), fit()});
    death_actions.set_content_alignment(Anchor::Center);
    death_actions.set_spacing(12.0F);
    death_actions.add<TextWidget>("you died");
    death_actions.add<MenuButton>("retry").on_click([] { game.restart_current_level(); });
    death_actions.add<MenuButton>("main menu").on_click([] { game.return_to_menu(); });
}
