#include "game-ui.hpp"
#include "../game.hpp"
#include "overlays/option-layer.hpp"
#include "overlays/exit.hpp"
#include "overlays/editor.hpp"
#include "overlays/level-selector.hpp"
#include "overlays/option-layer.hpp"
#include "overlays/settings.hpp"
#include "widgets/menu-button.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

GameUI::GameUI() : LayerContainer("game-ui") {
    set_size({grow(), grow()});

    // build menu
    {
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
            actions.add<MenuButton>(text).on_click([this, screen] {
                if (screen == GameScreen::Levels || screen == GameScreen::Editor) {
                    show_levels(screen == GameScreen::Editor);
                    return;
                }

                show_screen(screen);
            });
        }

        menu.set_visible(true);
        m_open.push_back(GameScreen::Menu);
    }

    // build options
    {
        m_screens.emplace(GameScreen::Levels, &add<LevelSelectorLayer>("levels", [this](DashLevel& level) {
                              play_level(level, m_edit_selected_level);
                          }));
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

    // build gameplay
    {
        auto& gameplay = add<LayerContainer>("gameplay");
        m_screens.emplace(GameScreen::Gameplay, &gameplay);
        gameplay.set_size({grow(), grow()});
        gameplay.set_input_mode(InputMode::None);
        gameplay.set_enabled(false);
        gameplay.set_visible(false);

        m_screens.emplace(GameScreen::Editor, &add<EditorLayer>());

        auto& pause = add<MenuOptionLayer>("pause");
        m_screens.emplace(GameScreen::Pause, &pause);
        pause.set_content_alignment(Anchor::Center);
        auto& pause_actions = pause.add<Container>("pause-actions");
        pause_actions.set_size({fit(), fit()});
        pause_actions.set_content_alignment(Anchor::Center);
        pause_actions.set_spacing(12.0F);
        pause_actions.add<TextWidget>("paused");
        pause_actions.add<MenuButton>("resume").on_click([this] {
            if (game.level_state() == LevelState::EDITING) {
                hide_screen(GameScreen::Pause);
                return;
            }

            game.resume_level();
        });
        pause_actions.add<MenuButton>("settings").on_click([this] { show_screen(GameScreen::Settings); });
        pause_actions.add<MenuButton>("main menu").on_click([] { game.return_to_menu(); });

        auto& death = add<MenuOptionLayer>("death", TransitionSpec{0.15F, easing::out_cubic}, false);
        m_screens.emplace(GameScreen::Death, &death);
        death.set_size({grow(), grow()});
        death.set_content_alignment(Anchor::Center);
        death.configure_all_styles([](Style& style) {
            style.background_color(rgba(160, 0, 0, 85));
            style.border(0);
        });

        auto& death_actions = death.add<Container>("death-actions");
        death_actions.set_size({fit(), fit()});
        death_actions.set_content_alignment(Anchor::Center);
        death_actions.set_spacing(12.0F);
        death_actions.add<TextWidget>("you died");
        death_actions.add<MenuButton>("retry").on_click([] { game.restart_current_level(); });
        death_actions.add<MenuButton>("main menu").on_click([] { game.return_to_menu(); });
    }

    // blocking panels receive keyboard input before the root of the ui tree.
    for (const auto& entry : m_screens) {
        if (auto* layer = dynamic_cast<MenuOptionLayer*>(entry.second)) {
            layer->on_key_press([this](UiEvent& event) { this->event(event); });
        }
    }
}

GameScreen GameUI::focused() const {
    return m_open.empty() ? GameScreen::Gameplay : m_open.back();
}

bool GameUI::pointer_over_ui() const {
    for (const auto& [screen, node] : m_screens) {
        if (!node->visible() || !node->enabled() || screen == GameScreen::Gameplay) continue;

        if (screen == GameScreen::Editor) {
            for (const auto& child : node->children()) {
                if (child->visible() && child->enabled() && child->subtree_input_state().hovered) return true;
            }
        } else if (node->subtree_input_state().hovered) {
            return true;
        }
    }

    return false;
}

void GameUI::focus_current() {
    surface().input_router().set_focus(m_open.empty() ? nullptr : m_screens.at(m_open.back()));
}

void GameUI::show_screen(GameScreen screen) {
    if (screen == GameScreen::Menu || screen == GameScreen::Gameplay || screen == GameScreen::Editor) {
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

void GameUI::show_levels(bool editor) {
    m_edit_selected_level = editor;
    show_screen(GameScreen::Levels);
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

void GameUI::play_level(DashLevel& level, bool editor) {
    auto& loading = *m_screens.at(GameScreen::Loading);
    if (loading.visible()) return;

    // load objects, player and music behind the loading screen before starting gameplay.
    add(detach(loading));

    loading.set_visible(true);
    loading.animator().animate().delay(0.25F).end([this, &level, &loading, editor] {
        hide_screen(GameScreen::Menu);
        hide_screen(GameScreen::Levels);
        show_screen(GameScreen::Loading);

        loading.animator().animate().delay(0.2F).end([&level, &loading, editor] {
            const bool loaded = game.load_level(level, editor);
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
    std::cout << "screen: " << static_cast<int>(screen) << "\n";
    if (screen == GameScreen::Menu || screen == GameScreen::Loading || screen == GameScreen::Death) {
        return;
    }

    if (screen == GameScreen::Gameplay) {
        if (escape) game.pause_level();
    } else if (screen == GameScreen::Editor) {
        if (escape) show_screen(GameScreen::Pause);
    } else if (screen == GameScreen::Pause) {
        if (game.level_state() == LevelState::EDITING) {
            hide_screen(GameScreen::Pause);
        } else {
            game.resume_level();
        }
    } else {
        hide_screen(screen);
    }

    event.stop_propagation();
    event.block_native_input();
}
