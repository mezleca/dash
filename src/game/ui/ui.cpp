#include "ui.hpp"
#include "controls.hpp"
#include "theme.hpp"
#include "game/dash.hpp"
#include "overlays/overlay.hpp"
#include "overlays/exit/exit.hpp"
#include "overlays/editor/editor.hpp"
#include "overlays/level-selector/level-selector.hpp"
#include "overlays/settings/settings.hpp"

#include <imgui-ui/runtime.hpp>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/backends/opengl/texture-loader.hpp>
#include <imgui-ui/backends/raylib/backend.hpp>
#include <imgui-ui/diagnostics/debugger.hpp>
#include <imgui-ui/layout/layer-container.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <array>
#include <unordered_map>
#include <utility>
#include <vector>
#include <raylib.h>

using namespace ui;

class MenuButton : public DashButton {
public:
    explicit MenuButton(std::string text);

protected:
    void apply_theme_defaults(const Theme& theme) override;
};

MenuButton::MenuButton(std::string text) : DashButton(std::move(text)) {}

void MenuButton::apply_theme_defaults(const Theme& theme) {
    DashButton::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 28);
    const TransitionSpec transition{0.2F, easing::out_cubic};

    configure_all_styles([&transition](Style& style) {
        style.background_color(rgba(0, 0, 0, 0), transition);
        style.border(BorderEdge{0.0F, BorderType::None}).border_bottom(1.0F);
        style.border_radius(0);
        style.border_color(rgb(200, 200, 200), transition);
        style.padding({12.0F, 2.0F});
    });

    style(StyleType::HOVER).border_color(theme.accent_color, transition);
}

class GameScreens : public LayerContainer {
public:
    GameScreens(Dash& game, Surface& ui_surface);

    void show_screen(GameScreen screen);
    bool pointer_over_ui(ImVec2 position) const;
    void request_editor_exit();
    bool editor_active() const;

    void event(UiEvent& event) override;

private:
    GameScreen focused() const;
    void show_levels(bool editor);
    void hide_screen(GameScreen screen);
    void hide_all_screens();
    void focus_current();
    void play_level(DashLevel& level, bool editor);
    void finish_level_loading(bool loaded);

    Dash& m_game;
    std::unordered_map<GameScreen, Container*> m_screens;
    std::vector<GameScreen> m_open;
    bool m_edit_selected_level = false;
};

class ResultOverlay : public Overlay {
public:
    ResultOverlay(Dash& game, GameScreens& screens, bool completed);
    void show() override;

private:
    Container* m_actions = nullptr;
    float m_content_delay;
};

ResultOverlay::ResultOverlay(Dash& game, GameScreens& screens, bool completed)
    : Overlay(completed ? "finished" : "death", {0.35F, easing::out_cubic}, false), m_content_delay(completed ? 0.35F : 1.0F) {
    set_size({grow(), grow()});
    set_content_alignment(Anchor::Center);
    configure_all_styles([completed](Style& style) {
        style.background_color(completed ? rgba(0, 110, 60, 100) : rgba(160, 0, 0, 85));
        style.border(BorderEdge{0.0F, BorderType::None});
    });

    m_actions = &add<Container>(completed ? "finished-actions" : "death-actions");
    m_actions->set_size({fit(), fit()});
    m_actions->set_content_alignment(Anchor::Center);
    m_actions->set_spacing(12.0F);
    m_actions->add<TextWidget>(completed ? "level completed!" : "you died");
    m_actions->add<MenuButton>(completed ? "play again" : "retry").on_click([&game, &screens] {
        if (game.restart_current_level()) {
            screens.show_screen(GameScreen::Gameplay);
        }
    });

    m_actions->add<MenuButton>("main menu").on_click([&game] { game.return_to_menu(); });
}

void ResultOverlay::show() {
    Overlay::show();

    // reset before the next fade so transitions replay after retrying a level
    set_opacity(0.0F, {});
    animator().animate().end([this] { fade_in({0.35F, easing::out_cubic}); });

    m_actions->cancel_animations();
    m_actions->set_visible(true);
    m_actions->set_opacity(0.0F, {});
    m_actions->set_input_enabled(false);

    m_actions->animator().animate().delay(m_content_delay).end([this] {
        m_actions->fade_in({0.3F, easing::out_cubic});
        m_actions->set_input_enabled(true);
    });
}

GameScreens::GameScreens(Dash& game, Surface& ui_surface) : LayerContainer("game-ui"), m_game(game) {
    set_size({grow(), grow()});

    // build menu
    {
        auto& menu = add<Container>("main-menu");
        m_screens.emplace(GameScreen::Menu, &menu);
        menu.set_size({grow(), grow()});
        menu.set_content_alignment(Anchor::Center);
        menu.configure_all_styles([&ui_surface](Style& style) {
            style.background_color(ui_surface.theme().background_color);
            style.border_radius(0.0f);
        });
        menu.add<TextWidget>("DASH").set_font(ui_surface.get_primary_font(), 56);

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
        m_screens.emplace(GameScreen::Levels, &add<LevelSelectorOverlay>(m_game, "levels", [this](DashLevel& level) {
                              play_level(level, m_edit_selected_level);
                          }));
        m_screens.emplace(GameScreen::Settings, &add<SettingsOverlay>(m_game, "game"));
        m_screens.emplace(
            GameScreen::Exit,
            &add<ExitOverlay>("game", "exit game?", [this] { surface().exit(); }, [this] { hide_screen(GameScreen::Exit); })
        );
        m_screens.emplace(
            GameScreen::EditorExit, &add<ExitOverlay>(
                                        "editor", "discard unsaved changes and leave?", [this] { m_game.return_to_menu(); },
                                        [this] { hide_screen(GameScreen::EditorExit); }
                                    )
        );

        auto& loading = add<LayerContainer>("level-loading");
        m_screens.emplace(GameScreen::Loading, &loading);

        loading.set_size({grow(), grow()});
        loading.set_content_alignment(Anchor::BottomCenter);
        loading.set_input_mode(InputMode::Blocker);
        loading.set_input_enabled(false);
        loading.set_visible(false);
        loading.add<TextWidget>("loading...").set_font(ui_surface.get_primary_font(), 32);
    }

    // build gameplay
    {
        auto& gameplay = add<LayerContainer>("gameplay");
        m_screens.emplace(GameScreen::Gameplay, &gameplay);
        gameplay.set_size({grow(), grow()});
        gameplay.set_input_mode(InputMode::None);
        gameplay.set_input_enabled(false);
        gameplay.set_visible(false);

        m_screens.emplace(
            GameScreen::Editor,
            &add<EditorOverlay>(
                m_game, ui_surface, [this](UiEvent& event) { this->event(event); }, [this] { request_editor_exit(); }
            )
        );

        m_screens.emplace(GameScreen::Death, &add<ResultOverlay>(m_game, *this, false));
        m_screens.emplace(GameScreen::Finished, &add<ResultOverlay>(m_game, *this, true));
    }

    // blocking panels receive keyboard input before the root of the ui tree.
    for (const auto& entry : m_screens) {
        if (auto* layer = dynamic_cast<Overlay*>(entry.second)) {
            layer->on_key_press([this](UiEvent& event) { this->event(event); });
        }
    }
}

GameScreen GameScreens::focused() const {
    return m_open.empty() ? GameScreen::Gameplay : m_open.back();
}

bool GameScreens::editor_active() const {
    return focused() == GameScreen::Editor;
}

void GameScreens::request_editor_exit() {
    auto* editor = m_game.editor();
    if (editor == nullptr || m_game.level_state() != LevelState::EDITING) {
        return;
    }

    editor->pointer_up();
    const auto& overlay = static_cast<const EditorOverlay&>(*m_screens.at(GameScreen::Editor));
    if (overlay.has_unsaved_changes()) {
        show_screen(GameScreen::EditorExit);
    } else {
        m_game.return_to_menu();
    }
}

bool GameScreens::pointer_over_ui(ImVec2 position) const {
    const ui::Node* target = surface().input_router().node_at(position, EventType::PointerDown);
    if (target == nullptr) {
        return false;
    }

    for (const auto& [screen, node] : m_screens) {
        if (!node->visible() || !node->input_enabled() || screen == GameScreen::Gameplay) {
            continue;
        }

        if (screen == GameScreen::Editor) {
            for (const auto& child : node->children()) {
                if (child->visible() && child->input_enabled() && child->contains(target)) {
                    return true;
                }
            }
        } else if (node->contains(target)) {
            return true;
        }
    }

    return false;
}

void GameScreens::focus_current() {
    surface().input_router().set_focus(m_open.empty() ? nullptr : m_screens.at(m_open.back()));
}

void GameScreens::show_screen(GameScreen screen) {
    if (screen == GameScreen::Menu || screen == GameScreen::Gameplay || screen == GameScreen::Editor) {
        hide_all_screens();
    }

    auto& node = *m_screens.at(screen);
    if (screen == GameScreen::EditorExit) {
        m_screens.at(GameScreen::Editor)->set_input_enabled(false);
    }
    if (screen == GameScreen::Editor) {
        static_cast<EditorOverlay&>(node).reset();
    }
    add(detach(node));

    if (auto* panel = dynamic_cast<Overlay*>(&node)) {
        if (!panel->input_enabled()) panel->show();
    } else {
        node.set_input_enabled(true);
        node.set_visible(true);
    }

    std::erase(m_open, screen);
    m_open.push_back(screen);
    focus_current();
}

void GameScreens::show_levels(bool editor) {
    m_edit_selected_level = editor;
    show_screen(GameScreen::Levels);
}

void GameScreens::hide_screen(GameScreen screen) {
    auto& node = *m_screens.at(screen);
    if (screen == GameScreen::EditorExit) {
        m_screens.at(GameScreen::Editor)->set_input_enabled(true);
    }

    if (auto* panel = dynamic_cast<Overlay*>(&node)) {
        panel->hide();
    } else {
        node.cancel_animations();
        node.set_input_enabled(false);
        node.set_visible(false);
    }

    std::erase(m_open, screen);
    focus_current();
}

void GameScreens::hide_all_screens() {
    for (const auto& entry : m_screens) {
        auto& node = *entry.second;
        node.cancel_animations();
        node.set_input_enabled(false);
        node.set_visible(false);
    }

    m_open.clear();
    focus_current();
}

void GameScreens::play_level(DashLevel& level, bool editor) {
    auto& loading = *m_screens.at(GameScreen::Loading);
    if (loading.visible()) return;

    // shortcuts!!
    const bool force_free_mode = !editor && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));

    // delays let the loading screen render before the synchronous load blocks the main thread
    add(detach(loading));

    loading.set_visible(true);
    loading.animator().animate().delay(0.25F).end([this, &level, &loading, editor, force_free_mode] {
        hide_screen(GameScreen::Menu);
        hide_screen(GameScreen::Levels);
        show_screen(GameScreen::Loading);

        loading.animator().animate().delay(0.2F).end([this, &level, &loading, editor, force_free_mode] {
            const bool loaded = m_game.load_level(level, editor, force_free_mode);
            loading.animator().animate().delay(0.25F).end([this, loaded] { finish_level_loading(loaded); });
        });
    });
}

void GameScreens::finish_level_loading(bool loaded) {
    if (loaded && m_game.start_level()) {
        show_screen(m_game.level_state() == LevelState::EDITING ? GameScreen::Editor : GameScreen::Gameplay);
        return;
    }

    if (loaded) m_game.unload_current_level();
    show_screen(GameScreen::Menu);
    show_screen(GameScreen::Levels);
}

void GameScreens::event(UiEvent& event) {
    const bool escape = event.type == EventType::KeyDown && event.key == Key::Escape;
    const bool backdrop = event.type == EventType::Click && event.target == this;
    if (!escape && !backdrop) {
        return;
    }

    const GameScreen screen = focused();
    if (screen == GameScreen::Menu || screen == GameScreen::Loading || screen == GameScreen::Death ||
        screen == GameScreen::Finished) {
        return;
    }

    if (screen == GameScreen::Gameplay) {
        return;
    }

    if (screen == GameScreen::Editor) {
        if (escape) {
            request_editor_exit();
        } else {
            return;
        }
    } else {
        hide_screen(screen);
    }

    event.stop_propagation();
    event.block_native_input();
}

GameUI::GameUI(Dash& game) : m_game(game) {
    RuntimeConfig runtime_config;
    runtime_config.texture_loader = std::make_unique<OpenGLTextureLoader>();
    m_runtime = std::make_unique<Runtime>(std::move(runtime_config));

    SurfaceConfig config;
    config.backend = std::make_unique<RaylibBackend>();
    config.enable_debugger = true;
    m_surface = std::make_unique<Surface>(*m_runtime, std::move(config));

    auto* font = m_runtime->fonts().add("MainFont", "resources/fonts/Baloo-Regular.ttf");
    m_surface->set_theme(make_dash_theme());
    m_surface->set_primary_font(font);
    m_surface->debugger()->set_font("MainFont", 24);

    auto& root = m_surface->root();
    static_cast<Container&>(root).configure_all_styles([](Style& style) { style.background_color(rgba(0, 0, 0, 0)); });
    m_screens = &root.add<GameScreens>(game, *m_surface);

    m_surface->input_router().on_event([this](UiEvent& event) {
        // handle escape before focused text inputs can consume it
        if (event.type == EventType::KeyDown && event.key == Key::Escape) {
            m_screens->event(event);
            if (event.propagation_stopped) return;
        }

        auto* editor = m_game.editor();
        if (event.type != EventType::PointerDown || event.button != PointerButton::Left ||
            m_game.level_state() != LevelState::EDITING || editor == nullptr || m_surface->debugger()->is_open() ||
            !m_screens->editor_active() || m_screens->pointer_over_ui(event.position)) {
            return;
        }

        auto* viewport = m_game.get_viewport(0);
        if (viewport != nullptr && viewport->contains_mouse()) {
            editor->pointer_down(viewport->mouse_position());
        }
    });
}

GameUI::~GameUI() = default;

bool GameUI::process_events() {
    m_surface->process_events();
    return !m_surface->is_done();
}

void GameUI::update_editor() {
    auto* editor = m_game.editor();
    if (editor != nullptr && !m_screens->editor_active()) {
        editor->pointer_up();
        return;
    }
    if (editor != nullptr && m_game.level_state() == LevelState::EDITING) {
        const Vector2 pointer = GetMousePosition();
        editor->update(m_screens->pointer_over_ui({pointer.x, pointer.y}), m_surface->debugger()->is_open());
    }
}

void GameUI::draw(float frametime) {
    m_surface->begin_frame();

    for (std::size_t index = 0; index < m_game.viewport_count(); ++index) {
        if (auto* viewport = m_game.get_viewport(index)) {
            viewport->draw();
        }
    }

    m_surface->update(frametime);
    m_surface->draw();
    m_surface->end_frame();
}

void GameUI::show_screen(GameScreen screen) {
    m_screens->show_screen(screen);
}
