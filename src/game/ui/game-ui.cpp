#include "game-ui.hpp"
#include "overlays/option-layer.hpp"
#include "../game.hpp"

#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

GameScreen GameUI::focused() const {
    return m_open.empty() ? GameScreen::Gameplay : m_open.back();
}

void GameUI::focus_current() {
    surface().input_router().set_focus(m_open.empty() ? nullptr : m_screens.at(m_open.back()));
}

void GameUI::show_screen(GameScreen screen) {
    if (screen == GameScreen::Menu || screen == GameScreen::Gameplay) {
        hide_all_screens();
    }

    auto& node = *m_screens.at(screen);
    add(detach(node));

    if (auto* panel = dynamic_cast<MenuOptionLayer*>(&node)) {
        if (!panel->enabled()) panel->show();
    } else {
        node.set_enabled(true);
        node.set_visible(true);
    }

    std::erase(m_open, screen);
    m_open.push_back(screen);
    focus_current();
}

void GameUI::hide_screen(GameScreen screen) {
    auto& node = *m_screens.at(screen);

    if (auto* panel = dynamic_cast<MenuOptionLayer*>(&node)) {
        panel->hide();
    } else {
        node.cancel_animations();
        node.set_enabled(false);
        node.set_visible(false);
    }

    std::erase(m_open, screen);
    focus_current();
}

void GameUI::hide_all_screens() {
    for (const auto& entry : m_screens) {
        auto& node = *entry.second;
        node.cancel_animations();
        node.set_enabled(false);
        node.set_visible(false);
    }

    m_open.clear();
    focus_current();
}

void GameUI::play_level(DashLevel& level) {
    auto& loading = *m_screens.at(GameScreen::Loading);
    if (loading.visible()) return;

    // show the screen while cards stay clickable for 250 ms. then fade the selector, load the level,
    // and keep the label visible for another 250 ms before gameplay starts.
    add(detach(loading));

    loading.set_visible(true);
    loading.animator().animate().delay(0.25F).end([this, &level, &loading] {
        hide_screen(GameScreen::Menu);
        hide_screen(GameScreen::Levels);
        show_screen(GameScreen::Loading);

        loading.animator().animate().delay(0.2F).end([&level, &loading] {
            const bool loaded = game.load_level(level);
            loading.animator().animate().delay(0.25F).end([loaded] { game.finish_level_loading(loaded); });
        });
    });
}

void GameUI::event(UiEvent& event) {
    const bool escape = event.type == EventType::KeyDown && event.key == Key::Escape;
    const bool backdrop = event.type == EventType::Click && event.target == this;
    if (!escape && !backdrop) {
        return;
    }

    const GameScreen screen = focused();
    if (screen == GameScreen::Menu || screen == GameScreen::Loading || screen == GameScreen::Death) {
        return;
    }

    if (screen == GameScreen::Gameplay) {
        if (escape) game.pause_level();
    } else if (screen == GameScreen::Pause) {
        game.resume_level();
    } else {
        hide_screen(screen);
    }

    event.stop_propagation();
    event.block_native_input();
}
