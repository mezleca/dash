#include "ui.hpp"
#include "screens.hpp"
#include "theme.hpp"
#include "game/dash.hpp"

#include <imgui-ui/runtime.hpp>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/backends/opengl/texture-loader.hpp>
#include <imgui-ui/backends/raylib/backend.hpp>
#include <imgui-ui/diagnostics/debugger.hpp>
#include <utility>

using namespace ui;

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
    m_surface->input_router().on_event([this](UiEvent& event) { m_screens->process_input(event); });
}

GameUI::~GameUI() = default;

bool GameUI::process_events() {
    m_surface->process_events();

    return !m_surface->is_done();
}

void GameUI::update_editor() {
    m_screens->update_editor();
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
