#include "game-ui.hpp"
#include "overlays/option-layer.hpp"
#include "../game.hpp"

#include <imgui-ui/surface.hpp>

#include <algorithm>
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
    if (screen == GameScreen::Menu || screen == GameScreen::Gameplay) {
        show_base(screen);
        return;
    }

    if (screen == focused()) return;

    if (screen == GameScreen::Loading) {
        show_loading();
        return;
    }

    if (focused() == GameScreen::Loading) {
        hide_loading();
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

void GameUI::show_base(GameScreen screen) {
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
}

void GameUI::show_loading() {
    m_open.push_back(GameScreen::Loading);
    m_menu->set_visible(false);
    if (m_levels->visible()) m_levels->hide(false);

    // loading takes input after the selector's fade begins.
    if (!m_loading->visible()) bring_to_front(*m_loading);
    m_loading->set_enabled(true);
    m_loading->set_input_mode(InputMode::Blocker);
    m_loading->set_visible(true);
    focus_current();
}

void GameUI::hide_loading() {
    m_open.pop_back();
    m_loading->animator().cancel();
    m_loading->set_enabled(false);
    m_loading->set_visible(false);
    m_loading->set_input_mode(InputMode::None);
    m_menu->set_visible(m_base == GameScreen::Menu);
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
