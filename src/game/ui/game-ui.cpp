#include "game-ui.hpp"
#include "overlays/exit.hpp"
#include "overlays/level-selector.hpp"
#include "overlays/settings.hpp"
#include "widgets/menu-button.hpp"
#include "../game.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>

#include <algorithm>
#include <array>
#include <utility>

using namespace ui;

MenuOptionLayer* GameUI::option(GameScreen screen) const {
    switch (screen) {
        case GameScreen::Levels:
            return m_levels;
        case GameScreen::Editor:
            return m_editor;
        case GameScreen::Settings:
            return m_settings;
        case GameScreen::Exit:
            return m_exit;
        case GameScreen::Pause:
            return m_pause;
        case GameScreen::Death:
            return m_death;
        default:
            return nullptr;
    }
}

GameScreen GameUI::focused() const {
    return m_open.empty() ? m_base : m_open.back();
}

bool GameUI::is_open(GameScreen screen) const {
    if (screen == GameScreen::Menu) return m_menu->visible();
    if (screen == GameScreen::Gameplay) return m_base == GameScreen::Gameplay;
    return std::find(m_open.begin(), m_open.end(), screen) != m_open.end();
}

void GameUI::bring_to_front(Node& node) {
    auto child = detach(node);
    add(std::move(child));
}

void GameUI::focus_current() {
    Node* target = nullptr;
    if (m_open.empty()) {
        if (m_base == GameScreen::Menu) target = m_menu;
    } else if (m_open.back() == GameScreen::Loading) {
        target = m_loading;
    } else {
        target = option(m_open.back());
    }

    surface().input_router().set_focus(target);
}

void GameUI::show(GameScreen screen) {
    // close every overlay and restore the menu visibility before focusing a base screen.
    if (screen == GameScreen::Menu || screen == GameScreen::Gameplay) {
        for (MenuOptionLayer* layer : {m_levels, m_editor, m_settings, m_exit, m_pause, m_death}) {
            layer->cancel_animations();
            layer->set_enabled(false);
            layer->set_visible(false);
        }

        m_loading->animator().cancel();
        m_loading->set_enabled(false);
        m_loading->set_visible(false);
        m_loading->set_input_mode(InputMode::None);
        m_menu->set_visible(screen == GameScreen::Menu);
        m_open.clear();
        m_base = screen;
        focus_current();
        return;
    }

    if (screen == focused()) return;

    // loading takes input after the selector's fade begins. the loading label is already visible above its cards.
    if (screen == GameScreen::Loading) {
        m_open.push_back(screen);
        m_menu->set_visible(false);
        if (m_levels->visible()) m_levels->hide(false);

        if (!m_loading->visible()) bring_to_front(*m_loading);
        m_loading->set_enabled(true);
        m_loading->set_input_mode(InputMode::Blocker);
        m_loading->set_visible(true);
        focus_current();
        return;
    }

    if (focused() == GameScreen::Loading) {
        m_open.pop_back();
        m_loading->animator().cancel();
        m_loading->set_enabled(false);
        m_loading->set_visible(false);
        m_loading->set_input_mode(InputMode::None);
        m_menu->set_visible(m_base == GameScreen::Menu);
    }

    MenuOptionLayer* layer = option(screen);
    if (layer == nullptr) return;

    auto existing = std::find(m_open.begin(), m_open.end(), screen);
    if (existing != m_open.end()) m_open.erase(existing);

    bring_to_front(*layer);
    if (!layer->visible() || !layer->enabled()) layer->show();
    m_open.push_back(screen);
    focus_current();
}

void GameUI::play_level(DashLevel& level) {
    if (m_loading->visible()) return;

    // show the label while cards stay clickable for 250 ms. then fade the selector, load the level,
    // and keep the label visible for another 250 ms before gameplay starts.
    bring_to_front(*m_loading);
    m_loading->set_enabled(false);
    m_loading->set_visible(true);
    m_loading->animator().animate().delay(0.25F).end([this, &level] {
        show(GameScreen::Loading);
        m_loading->animator().animate().delay(0.2F).end([this, &level] { load_level(level); });
    });
}

void GameUI::load_level(DashLevel& level) {
    const bool loaded = game.load_level(level);
    m_loading->animator().animate().delay(0.25F).end([loaded] { game.finish_level_loading(loaded); });
}

void GameUI::close_panel() {
    if (m_open.empty() || focused() == GameScreen::Loading || focused() == GameScreen::Death) return;
    if (focused() == GameScreen::Pause) {
        game.resume_level();
        return;
    }

    MenuOptionLayer* layer = option(m_open.back());
    m_open.pop_back();
    layer->hide();
    focus_current();
}

void GameUI::event(UiEvent& event) {
    const bool escape = event.type == EventType::KeyDown && event.key == Key::Escape;
    const bool backdrop = event.type == EventType::Click && event.target == this;
    if (!escape && !backdrop) return;

    const GameScreen screen = focused();
    if (screen == GameScreen::Menu || screen == GameScreen::Loading || screen == GameScreen::Death) return;

    if (screen == GameScreen::Gameplay) {
        if (!escape) return;
        game.pause_level();
    } else {
        close_panel();
    }

    event.stop_propagation();
    event.block_native_input();
}

GameUI::GameUI() : LayerContainer("game-ui") {
    set_size({grow(), grow()});

    // build main menu
    auto& menu = add<Container>("main-menu");
    m_menu = &menu;
    menu.set_size({grow(), grow()});
    menu.set_content_alignment(Anchor::Center);
    menu.add<TextWidget>("DASH").set_font(game.m_ui->get_primary_font(56));

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

    // build option panels
    m_levels = &add<LevelSelectorLayer>("play", [this](DashLevel& level) { play_level(level); });
    m_editor = &add<LevelSelectorLayer>("editor");
    m_settings = &add<SettingsLayer>("game");
    m_exit = &add<ExitLayer>("game", [this] { close_panel(); });

    // build pause and death screens
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

    // draw loading above the selector without taking its input before the fade starts.
    m_loading = &add<LayerContainer>("level-loading");
    m_loading->set_size({grow(), grow()});
    m_loading->set_content_alignment(Anchor::BottomCenter);
    m_loading->set_input_mode(InputMode::None);
    m_loading->set_enabled(false);
    m_loading->set_visible(false);
    m_loading->add<TextWidget>("loading...").set_font(game.m_ui->get_primary_font(32));

    // blocking panels receive keyboard input before the root of the ui tree.
    for (MenuOptionLayer* layer : {m_levels, m_editor, m_settings, m_exit, m_pause, m_death}) {
        layer->on_key_press([this](UiEvent& event) { this->event(event); });
    }
}
