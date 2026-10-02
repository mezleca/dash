#include "game.hpp"
#include "ui/game-ui.hpp"
#include "../utils/math.hpp"

#include <imgui-ui/runtime.hpp>
#include <imgui-ui/diagnostics/debugger.hpp>
#include <imgui-ui/backends/raylib/backend.hpp>
#include <imgui-ui/layout/container.hpp>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <raylib.h>

static constexpr float CAMERA_PLATFORM_X_THRESHOLD = 200.0f;
static constexpr float CAMERA_PLATFORM_Y_THRESHOLD = 1000.0f;
static constexpr float CAMERA_Y_SMOOTHING = 0.09f;
static constexpr float CAMERA_X_LOOK_AHEAD = 128.0f;
static constexpr float CAMERA_Y_LOOK_AHEAD = -128.0f;

using namespace ui;

Game::Game() {
    m_window.title = "dash";
    m_window.width = 1280;
    m_window.height = 720;

    m_camera = {};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.2f;
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
    m_ui = std::make_unique<Surface>(m_runtime, std::move(ui_config));

    // read metadata before the selector creates cards. game objects are loaded when a card is opened.
    load_all_levels();
    build_ui();

    while (!m_ui->is_done()) {
        // fixed physics ticks run before frame callbacks and drawing.
        handle_pause_state();
        update_simulation_timestep();

        m_ui->process_events();

        if (IsWindowResized()) {
            m_window.width = GetScreenWidth();
            m_window.height = GetScreenHeight();
        }

        update_current_level_progress();

        if (m_current_level != nullptr && m_current_level->m_state != LevelState::LOADING) {
            const float frametime = GetFrameTime();

            // update level behaviours and remove completed ones before entity components run.
            m_current_level->update_behaviours(frametime);

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
    if (m_current_level == nullptr || m_current_level->m_state != LevelState::PLAYING) {
        m_accumulator = 0.0f;
        m_alpha = 0.0f;
        return;
    }

    m_accumulator += GetFrameTime();

    // carry unfinished tick time into the interpolation fraction below.
    while (m_accumulator >= m_fixed_frametime) {
        m_accumulator -= m_fixed_frametime;
        simulate();

        if (is_paused()) {
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
        pause_current_level_music();
    } else {
        resume_current_level_music();
    }

    m_was_paused = is_paused();
}

void Game::pause_current_level_music() {
    if (m_current_level == nullptr || !m_current_level->m_music_loaded) {
        return;
    }

    std::cout << "[game] pausing music\n";
    PauseMusicStream(m_current_level->music);
}

void Game::resume_current_level_music() {
    if (m_current_level == nullptr || !m_current_level->m_music_loaded) {
        return;
    }

    std::cout << "[game] resuming music\n";
    SeekMusicStream(m_current_level->music, m_current_level->m_current_music_progress);
    ResumeMusicStream(m_current_level->music);
}

void Game::unload_current_level_music() {
    if (m_current_level == nullptr || !m_current_level->m_music_loaded) {
        return;
    }

    StopMusicStream(m_current_level->music);
    UnloadMusicStream(m_current_level->music);

    m_current_level->music = {};
    m_current_level->m_music_loaded = false;
}

void Game::update_current_level_progress() {
    if (is_paused() || m_current_level == nullptr || !m_current_level->m_music_loaded) {
        return;
    }

    UpdateMusicStream(m_current_level->music);

    m_current_level->m_current_music_progress = GetMusicTimePlayed(m_current_level->music);
    if (m_current_level->m_current_progress < 100.0f) {
        m_current_level->update();
    }
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
        std::cout << "[game] loaded level: " << level->m_name << "\n";
    }
}

bool Game::load_level(DashLevel& level) {
    if (m_current_level != nullptr) {
        std::cout << "[game] failed to load level while another level is active\n";
        return false;
    }

    if (level.m_objects.empty() && !level.m_temp_objects.empty()) {
        std::cout << "[game] loading data from level " << level.m_name << "\n";

        if (!level.load_objects(m_world)) {
            level.unload();
            std::cout << "[game] failed to load level from " << level.m_file << "\n";
            return false;
        }
    }

    m_current_level = &level;
    m_current_level->m_state = LevelState::LOADING;
    std::cout << "loaded " << level.m_file << " successfully\n";

    return true;
}

bool Game::start_level() {
    if (m_current_level == nullptr) {
        std::cout << "failed to start current level (not loaded)\n";
        return false;
    }

    if (m_player == nullptr) {
        m_player = std::make_unique<Player>(m_world, m_settings.godmode());
    } else {
        m_player->reset();
        m_player->set_god_mode(m_settings.godmode());
    }

    m_player->set_free_mode(m_free_mode);

    const Vector2 start = m_current_level->m_player_start;
    m_player->set_position(start.x, start.y);

    std::filesystem::path music_full_location = m_current_level->m_file.parent_path() / m_current_level->m_music_file;
    unload_current_level_music();

    m_current_level->music = LoadMusicStream(music_full_location.c_str());
    m_current_level->m_music_loaded = IsMusicValid(m_current_level->music);

    if (!m_current_level->m_music_loaded) {
        std::cout << "[game] failed to load music from " << music_full_location << "\n";
        return false;
    }

    SetMusicPan(m_current_level->music, 0.0f);
    SetMusicVolume(m_current_level->music, static_cast<float>(m_settings.volume()) / 100.0f);
    PlayMusicStream(m_current_level->music);

    m_settings.update_attempts(m_current_level->m_file.parent_path().filename().string());

    m_current_level->m_finished = false;
    m_current_level->m_state = LevelState::PLAYING;
    m_was_paused = false;
    m_game_ui->show_screen(GameScreen::Gameplay);

    return true;
}

void Game::unload_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to unload current level (not found)\n";
        return;
    }

    unload_current_level_music();
    m_current_level->unload();
    m_player.reset();

    m_was_paused = false;
    m_current_level = nullptr;
    m_game_ui->show_screen(GameScreen::Menu);
}

bool Game::restart_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to restart current level (not found)\n";
        return false;
    }

    m_current_level->m_behaviours.clear();
    m_current_level->m_current_progress = 0.0f;
    m_current_level->m_current_music_progress = 0.0f;

    return start_level();
}

void Game::finish_level_loading(bool loaded) {
    if (loaded && start_level()) return;

    if (loaded) unload_current_level();
    m_game_ui->show_screen(GameScreen::Menu);
    m_game_ui->show_screen(GameScreen::Levels);
}

void Game::set_paused(bool value) {
    if (m_current_level == nullptr || m_player == nullptr || !m_player->alive() || m_current_level->m_finished ||
        m_current_level->m_state == LevelState::LOADING) {
        return;
    }

    m_current_level->m_state = value ? LevelState::PAUSED : LevelState::PLAYING;
}

bool Game::is_paused() const {
    return m_current_level != nullptr && m_current_level->m_state == LevelState::PAUSED;
}

bool Game::has_finished_level() const {
    return m_current_level != nullptr && m_current_level->m_finished;
}

void Game::pause_level() {
    if (m_current_level == nullptr || m_current_level->m_state != LevelState::PLAYING) return;

    set_paused(true);

    if (is_paused()) {
        m_game_ui->show_screen(GameScreen::Pause);
    }
}

void Game::resume_level() {
    if (!is_paused()) return;

    set_paused(false);

    if (!is_paused()) {
        m_game_ui->show_screen(GameScreen::Gameplay);
    }
}

void Game::return_to_menu() {
    if (m_current_level != nullptr) {
        unload_current_level();
        return;
    }

    m_game_ui->show_screen(GameScreen::Menu);
}

void Game::finish_level() {
    if (m_current_level == nullptr || m_player == nullptr || !m_player->alive()) return;

    m_settings.set_progress(m_current_level->m_file.parent_path().filename().string(), 100);
    m_current_level->m_finished = true;
    m_current_level->m_state = LevelState::PAUSED;
}

void Game::kill_player() {
    if (m_current_level == nullptr || m_player == nullptr || !m_player->alive() || m_player->in_god_mode() ||
        has_finished_level()) {
        return;
    }

    std::cout << "[game] player died\n";
    m_settings.set_progress(
        m_current_level->m_file.parent_path().filename().string(), static_cast<int>(m_current_level->m_current_progress)
    );
    m_player->kill();
    m_current_level->m_state = LevelState::PAUSED;
    m_game_ui->show_screen(GameScreen::Death);
}

void Game::set_music_volume(int volume) {
    m_settings.set_volume(volume);

    if (m_current_level != nullptr && m_current_level->m_music_loaded) {
        SetMusicVolume(m_current_level->music, static_cast<float>(m_settings.volume()) / 100.0f);
    }
}

void Game::set_godmode(bool enabled) {
    m_settings.set_godmode(enabled);

    if (m_player != nullptr) {
        m_player->set_god_mode(enabled);
    }
}

void Game::set_free_mode(bool enabled) {
    m_free_mode = enabled;

    if (m_player != nullptr) {
        m_player->set_free_mode(enabled);
    }
}

void Game::update_camera_focus(Entity* obj) {
    const Rectangle bounds = obj->get_bounding_box();
    m_focus_y = bounds.y + bounds.height / 2.0f;
}

void Game::simulate() {
    if (m_player == nullptr) return;

    // player movement writes velocity before box2d advances the world.
    m_player->movement();
    m_world.step(m_fixed_frametime);

    const Vector2 player_position = m_player->get_position();
    const Rectangle player_bounds = m_player->get_bounding_box();
    Entity* closest_platform = nullptr;

    if (!has_finished_level()) {
        const float player_center_x = player_bounds.x + player_bounds.width / 2.0f;
        const float player_bottom_y = player_bounds.y + player_bounds.height;

        float closest_platform_distance = CAMERA_PLATFORM_Y_THRESHOLD;

        for (const auto& object : m_world.entities()) {
            if (object->type() != ObjectType::PLATFORM) continue;

            const Rectangle platform_bounds = object->get_bounding_box();
            const float platform_min_x = platform_bounds.x - CAMERA_PLATFORM_X_THRESHOLD;
            const float platform_max_x = platform_bounds.x + platform_bounds.width + CAMERA_PLATFORM_X_THRESHOLD;

            if (player_center_x < platform_min_x || player_center_x > platform_max_x) {
                continue;
            }

            const float platform_distance = std::fabs(player_bottom_y - platform_bounds.y);

            if (platform_distance >= closest_platform_distance) {
                continue;
            }

            closest_platform = object;
            closest_platform_distance = platform_distance;
        }

        if (closest_platform != nullptr) {
            update_camera_focus(closest_platform);
        } else {
            update_camera_focus(m_player.get());
        }
    }

    // follow the player horizontally and ease toward the selected vertical focus.
    m_camera.target = {
        player_position.x + CAMERA_X_LOOK_AHEAD,
        d_math::lerp(m_camera.target.y, m_focus_y + CAMERA_Y_LOOK_AHEAD, CAMERA_Y_SMOOTHING)
    };

    m_camera.offset = {static_cast<float>(m_window.width) / 2.0f, static_cast<float>(m_window.height) / 2.0f};
}

void Game::render() {
    const float frametime = GetFrameTime();
    m_ui->begin_frame();

    if (m_current_level != nullptr && m_current_level->m_state != LevelState::LOADING) {
        const auto& entities = m_world.entities();
        m_render_objects.assign(entities.begin(), entities.end());

        // stable order preserves load order when objects share a z_index.
        std::stable_sort(m_render_objects.begin(), m_render_objects.end(), [](const Entity* a, const Entity* b) {
            return a->z_index < b->z_index;
        });

        BeginMode2D(m_camera);
        {
            for (const auto& object : m_render_objects) {
                Rectangle bounds = object->get_bounding_box();

                // render the player between its positions before and after the last physics tick.
                if (object == m_player.get()) {
                    const Vector2 position = object->get_position();
                    const Vector2 previous = object->get_previous_position();
                    bounds.x += (previous.x - position.x) * (1.0f - m_alpha);
                    bounds.y += (previous.y - position.y) * (1.0f - m_alpha);
                }

                object->render(bounds, m_camera);
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
