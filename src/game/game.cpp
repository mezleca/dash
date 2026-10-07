#include "game.hpp"
#include "ui/game-ui.hpp"

#include <imgui-ui/runtime.hpp>
#include <imgui-ui/diagnostics/debugger.hpp>
#include <imgui-ui/backends/raylib/backend.hpp>
#include <imgui-ui/backends/opengl/texture-loader.hpp>
#include <imgui-ui/layout/container.hpp>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <raylib.h>

using namespace ui;

Game::Game() {
    m_window.title = "dash";
    m_window.width = 1280;
    m_window.height = 720;
}

void Game::initialize() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());
    InitAudioDevice();

    SetTargetFPS(60);
    SetExitKey(0);

    if (!m_settings.load()) {
        std::cerr << "[game] warning: settings could not be loaded\n";
    }

    auto backend = std::make_unique<RaylibBackend>();

    SurfaceConfig ui_config;
    ui_config.backend = std::move(backend);
    ui_config.enable_debugger = true;

    Runtime runtime({.texture_loader = std::make_unique<OpenGLTextureLoader>()});

    m_ui = std::make_unique<Surface>(runtime, std::move(ui_config));

    // read metadata before the selector creates cards. game objects are loaded when a card is opened.
    load_all_levels();
    build_ui();

    while (!m_ui->is_done()) {
        handle_pause_state();
        update_simulation_timestep();

        m_ui->process_events();
        m_pointer_over_ui = m_game_ui->pointer_over_ui();

        if (IsWindowResized()) {
            m_window.width = GetScreenWidth();
            m_window.height = GetScreenHeight();
        }

        if (m_current_level != nullptr && m_current_level->state() != LevelState::INVALID) {
            const float frametime = GetFrameTime();

            if (m_current_level->state() != LevelState::INVALID) {
                m_current_level->update();
            }

            // update level behaviours and remove completed ones before entity components run.
            if (m_current_level->state() != LevelState::DEATH) {
                m_current_level->update_behaviours(frametime);
            }

            for (auto* object : m_world.entities()) {
                object->update(frametime);
            }
        }

        render();
    }

    if (!m_settings.save()) {
        std::cerr << "[game] warning: settings could not be saved\n";
    }

    shutdown();
    CloseAudioDevice();
    CloseWindow();
}

void Game::build_ui() {
    auto& runtime = m_ui->runtime();
    auto* font = runtime.fonts().add("MainFont", "resources/fonts/Baloo-Regular.ttf");

    m_ui->set_primary_font(font);
    m_ui->debugger()->set_font("MainFont", 24);

    auto& root = m_ui->root();
    static_cast<Container&>(root).configure_all_styles([](Style& style) { style.background_color(rgba(0, 0, 0, 0)); });

    m_game_ui = &root.add<GameUI>();
}

void Game::update_simulation_timestep() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING) {
        m_accumulator = 0.0f;
        m_alpha = 0.0f;
        return;
    }

    m_accumulator += GetFrameTime();

    // carry unfinished tick time into the interpolation fraction below.
    while (m_accumulator >= m_fixed_frametime) {
        m_accumulator -= m_fixed_frametime;
        simulate();

        if (m_current_level->state() != LevelState::PLAYING) {
            m_accumulator = 0.0f;
            break;
        }
    }

    m_alpha = m_accumulator / m_fixed_frametime;
}

void Game::handle_pause_state() {
    if (is_paused() == m_was_paused) {
        return;
    }

    if (is_paused()) {
        m_accumulator = 0.0f;
        m_alpha = 0.0f;
    }

    m_was_paused = is_paused();
}

void Game::load_all_levels() {
    std::cout << "[game] loading levels...\n";

    for (const auto& entry : std::filesystem::directory_iterator(LEVELS_LOCATION)) {
        if (!entry.is_directory()) {
            continue;
        }

        for (const auto& file : std::filesystem::directory_iterator(entry)) {
            if (file.path().extension() != ".json") {
                continue;
            }

            const std::string location = file.path().string();
            std::cout << "[game] found level at " << location << "\n";

            auto level = std::make_unique<DashLevel>();
            if (!level->load(location)) {
                std::cout << "[game] failed to load level metadata from " << location << "\n";
                continue;
            }

            m_levels.push_back(std::move(level));
            break;
        }
    }

    for (const auto& level : m_levels) {
        std::cout << "[game] loaded level: " << level->name() << "\n";
    }
}

bool Game::load_level(DashLevel& level, bool editor) {
    if (m_current_level != nullptr) {
        std::cout << "[game] failed to load level while another level is active\n";
        return false;
    }

    level.set_state(editor ? LevelState::EDITING : LevelState::PAUSED);

    if (level.objects().empty()) {
        std::cout << "[game] loading data from level " << level.name() << "\n";

        if (!level.load_objects(m_world)) {
            level.unload();
            std::cout << "[game] failed to load level from " << level.file() << "\n";
            return false;
        }
    }

    m_camera.reset();

    if (editor) {
        m_camera.set_target(level.player_start());
    } else {
        level.spawn_player(m_world, m_settings.godmode(), m_free_mode);
        level.player()->update_camera(m_camera, m_world, false, true);
        level.set_music_volume(static_cast<float>(m_settings.volume()) / 100.0f);

        if (!level.load_music(false)) {
            level.unload();
            m_camera.set_target({});
            return false;
        }
    }

    m_current_level = &level;
    std::cout << "loaded " << level.file() << " successfully\n";

    return true;
}

bool Game::start_level() {
    if (m_current_level == nullptr) {
        std::cout << "failed to start current level (not loaded)\n";
        return false;
    }

    m_camera.reset();
    m_accumulator = 0.0f;
    m_alpha = 0.0f;

    m_current_level->reset();

    // configure editor
    if (m_current_level->state() == LevelState::EDITING) {
        // TOFIX: use latest editing position instead of player start
        m_camera.set_target(m_current_level->player_start());
        m_was_paused = false;
        m_game_ui->show_screen(GameScreen::Editor);
        return true;
    }

    // configure gameplayer
    m_current_level->spawn_player(m_world, m_settings.godmode(), m_free_mode);
    player()->update_camera(m_camera, m_world, false, true);
    m_current_level->set_music_volume(static_cast<float>(m_settings.volume()) / 100.0f);
    m_current_level->play_music();

    m_settings.update_attempts(m_current_level->file().parent_path().filename().string());

    m_current_level->set_state(LevelState::PLAYING);
    m_was_paused = false;
    m_game_ui->show_screen(GameScreen::Gameplay);

    return true;
}

void Game::unload_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to unload current level (not found)\n";
        return;
    }

    m_current_level->unload();

    m_camera.reset();
    m_camera.set_target({});
    m_accumulator = 0.0f;
    m_alpha = 0.0f;
    m_render_objects.clear();
    m_pointer_over_ui = false;
    m_was_paused = false;
    m_current_level = nullptr;
    m_game_ui->show_screen(GameScreen::Menu);
}

bool Game::restart_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to restart current level (not found)\n";
        return false;
    }

    return start_level();
}

void Game::finish_level_loading(bool loaded) {
    if (loaded && start_level()) return;

    if (loaded) unload_current_level();
    m_game_ui->show_screen(GameScreen::Menu);
    m_game_ui->show_screen(GameScreen::Levels);
}

void Game::set_paused(bool value) {
    if (m_current_level == nullptr || player() == nullptr || !player()->alive() || m_current_level->finished() ||
        m_current_level->state() == LevelState::EDITING || m_current_level->state() == LevelState::DEATH) {
        return;
    }

    m_current_level->set_state(value ? LevelState::PAUSED : LevelState::PLAYING);
}

bool Game::is_paused() const {
    return m_current_level != nullptr && m_current_level->state() == LevelState::PAUSED;
}

bool Game::has_finished_level() const {
    return m_current_level != nullptr && m_current_level->finished();
}

void Game::pause_level() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING) return;

    set_paused(true);
    m_game_ui->show_screen(GameScreen::Pause);
}

void Game::resume_level() {
    if (!is_paused()) return;

    set_paused(false);
    m_game_ui->show_screen(GameScreen::Gameplay);
}

void Game::return_to_menu() {
    if (m_current_level != nullptr) {
        unload_current_level();
        return;
    }

    m_game_ui->show_screen(GameScreen::Menu);
}

void Game::finish_level() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING || player() == nullptr ||
        !player()->alive())
        return;

    m_settings.set_progress(m_current_level->file().parent_path().filename().string(), 100);
    m_current_level->set_finished(true);
    m_current_level->set_state(LevelState::PAUSED);
}

void Game::kill_player() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING || player() == nullptr ||
        !player()->alive() || player()->in_god_mode() || has_finished_level()) {
        return;
    }

    std::cout << "[game] starting player death\n";

    m_settings.set_progress(
        m_current_level->file().parent_path().filename().string(), static_cast<int>(m_current_level->progress())
    );

    player()->freeze();
    m_current_level->begin_death();
    m_game_ui->show_screen(GameScreen::Death);
}

void Game::set_music_volume(int volume) {
    m_settings.set_volume(volume);
    if (m_current_level != nullptr) {
        m_current_level->set_music_volume(static_cast<float>(m_settings.volume()) / 100.0f);
    }
}

void Game::set_godmode(bool enabled) {
    m_settings.set_godmode(enabled);
    if (player() != nullptr) {
        player()->set_god_mode(enabled);
    }
}

void Game::set_free_mode(bool enabled) {
    m_free_mode = enabled;
    if (player() != nullptr) {
        player()->set_free_mode(enabled);
    }
}

void Game::simulate() {
    Player* current_player = player();
    if (current_player == nullptr) return;

    // player movement writes velocity before box2d advances the world.
    current_player->movement();
    m_world.step(m_fixed_frametime);

    // update camera focus to player
    if (m_current_level->state() == LevelState::PLAYING) {
        current_player->update_camera(m_camera, m_world, has_finished_level());
    }
}

void Game::render() {
    const float frametime = GetFrameTime();
    m_ui->begin_frame();
    m_camera.center_viewport({static_cast<float>(m_window.width), static_cast<float>(m_window.height)});

    if (m_current_level != nullptr && m_current_level->loaded()) {
        const auto& entities = m_world.entities();
        m_render_objects.assign(entities.begin(), entities.end());

        // stable order preserves load order when objects share a z_index.
        std::stable_sort(m_render_objects.begin(), m_render_objects.end(), [](const Entity* a, const Entity* b) {
            return a->z_index < b->z_index;
        });

        BeginMode2D(m_camera.transform());
        {
            for (const auto& object : m_render_objects) {
                Rectangle bounds = object->get_bounding_box();

                // render the player between its positions before and after the last physics tick.
                if (object == player()) {
                    const Vector2 position = object->get_position();
                    const Vector2 previous = object->get_previous_position();
                    bounds.x += (previous.x - position.x) * (1.0f - m_alpha);
                    bounds.y += (previous.y - position.y) * (1.0f - m_alpha);
                }

                object->render(bounds, m_camera.transform());
            }
        }
        EndMode2D();
    }

    // draw ui overlays after leaving the world camera.
    m_ui->update(frametime);
    m_ui->draw();
    m_ui->end_frame();
}

void Game::shutdown() {
    if (m_current_level != nullptr) {
        unload_current_level();
    }

    m_levels.clear();
    m_ui.reset();
    m_game_ui = nullptr;
}
