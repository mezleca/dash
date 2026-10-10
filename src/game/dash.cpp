#include "dash.hpp"
#include "ui/game-ui.hpp"
#include "core/math.hpp"

#include <imgui-ui/diagnostics/debugger.hpp>
#include <imgui-ui/runtime.hpp>
#include <imgui-ui/layout/container.hpp>
#include <iostream>
#include <raylib.h>

using namespace ui;

constexpr float DEATH_ZOOM_MULTIPLIER = 1.12f;
constexpr float DEATH_ROTATION_OFFSET = 5.0f;

Dash::Dash() : Game({"dash", 1280, 720}) {
    camera().view().zoom = DEFAULT_CAMERA_ZOOM;
    m_settings = std::make_unique<SettingsManager>(*this);
}

void Dash::on_initialize() {
    if (!m_settings->load()) {
        std::cerr << "[game] warning: settings could not be loaded\n";
    }

    // read metadata before the selector creates cards. game objects are loaded when a card is opened.
    for (const auto& entry : fs::directory_iterator(LEVELS_LOCATION)) {
        if (!entry.is_directory()) {
            continue;
        }

        const std::string id = entry.path().filename().string();
        for (const auto& file : fs::directory_iterator(entry)) {
            if (!Deserializer::supports_file(file.path())) {
                continue;
            }

            const auto& location = file.path();
            std::cout << "[game] found level at " << location << "\n";

            auto [level, inserted] = m_levels.try_emplace(id);
            if (!inserted) {
                break;
            }

            level->second = &add_object<DashLevel>(*this);
            if (!level->second->load(location)) {
                std::cout << "[game] failed to load level metadata from " << location << "\n";
                remove_object(level->second);
                m_levels.erase(level);
                continue;
            }

            break;
        }
    }

    m_settings->update_levels_progress();

    build_ui();
}

void Dash::on_update(float frametime) {
    if (m_current_level == nullptr || m_current_level->state() == LevelState::INVALID) {
        return;
    }

    if (m_current_level->state() == LevelState::EDITING) {
        m_editor_camera_controls.update(camera(), m_game_ui->pointer_over_ui());
    }

    if (m_current_level->update()) {
        finish_level();
    }

    if (m_current_level->state() == LevelState::DEATH) {
        const float progress = m_current_level->death_animation_progress();
        camera().view().zoom = d_math::lerp(DEFAULT_CAMERA_ZOOM, DEFAULT_CAMERA_ZOOM * DEATH_ZOOM_MULTIPLIER, progress);
        camera().view().rotation =
            d_math::lerp(DEFAULT_CAMERA_ROTATION, DEFAULT_CAMERA_ROTATION + DEATH_ROTATION_OFFSET, progress);
    }

    // update level behaviours and remove completed ones before entity components run.
    if (m_current_level->state() != LevelState::DEATH) {
        m_current_level->update_behaviours(frametime);
    }
}

void Dash::build_ui() {
    auto& runtime = surface().runtime();
    auto* font = runtime.fonts().add("MainFont", "resources/fonts/Baloo-Regular.ttf");

    surface().set_primary_font(font);
    surface().debugger()->set_font("MainFont", 24);

    auto& root = surface().root();
    static_cast<Container&>(root).configure_all_styles([](Style& style) { style.background_color(rgba(0, 0, 0, 0)); });

    m_game_ui = &root.add<GameUI>(*this);
}

bool Dash::simulation_active() const {
    return m_current_level != nullptr && m_current_level->state() == LevelState::PLAYING;
}

bool Dash::load_level(DashLevel& level, bool editor) {
    if (m_current_level != nullptr) {
        std::cout << "[game] failed to load level while another level is active\n";
        return false;
    }

    level.set_state(editor ? LevelState::EDITING : LevelState::PAUSED);

    if (!level.loaded()) {
        std::cout << "[game] loading data from level " << level.name() << "\n";

        if (!level.load_objects()) {
            level.unload();
            std::cout << "[game] failed to load level from " << level.file() << "\n";
            return false;
        }
    }

    camera().view().zoom = DEFAULT_CAMERA_ZOOM;
    camera().view().rotation = DEFAULT_CAMERA_ROTATION;
    m_editor_camera_controls.reset();

    if (editor) {
        camera().set_target(level.player_start());
    } else {
        level.spawn_player(m_settings->godmode(), free_mode());
        level.player()->update_camera(camera(), true);
        level.set_music_volume(static_cast<float>(m_settings->volume()) / 100.0f);

        if (!level.load_music()) {
            level.unload();
            camera().set_target({});
            return false;
        }
    }

    m_current_level = &level;
    std::cout << "loaded " << level.file() << " successfully\n";

    return true;
}

bool Dash::start_level() {
    if (m_current_level == nullptr) {
        std::cout << "failed to start current level (not loaded)\n";
        return false;
    }

    camera().view().zoom = DEFAULT_CAMERA_ZOOM;
    camera().view().rotation = DEFAULT_CAMERA_ROTATION;
    m_editor_camera_controls.reset();
    reset_simulation_clock();

    m_current_level->reset();

    // configure editor.
    if (m_current_level->state() == LevelState::EDITING) {
        // TOFIX: use latest editing position instead of player start.
        camera().set_target(m_current_level->player_start());
        return true;
    }

    // configure player and focus the camera.
    m_current_level->spawn_player(m_settings->godmode(), free_mode());
    player()->update_camera(camera(), true);

    // sync level with saved data.
    m_current_level->set_music_volume(static_cast<float>(m_settings->volume()) / 100.0f);
    m_current_level->play_music();
    m_current_level->record_attempt();

    m_settings->save();
    m_current_level->set_state(LevelState::PLAYING);
    return true;
}

void Dash::unload_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to unload current level (not found)\n";
        return;
    }

    m_current_level->unload();

    camera().view().zoom = DEFAULT_CAMERA_ZOOM;
    camera().view().rotation = DEFAULT_CAMERA_ROTATION;
    camera().set_target({});

    // reset editor camera to default values.
    m_editor_camera_controls.reset();

    reset_simulation_clock();
    m_current_level = nullptr;
}

bool Dash::restart_current_level() {
    if (m_current_level == nullptr) {
        std::cout << "[game] failed to restart current level (not found)\n";
        return false;
    }

    return start_level();
}

void Dash::set_paused(bool value) {
    if (m_current_level == nullptr || player() == nullptr || !player()->alive() || m_current_level->finished() ||
        m_current_level->state() == LevelState::EDITING || m_current_level->state() == LevelState::DEATH) {
        return;
    }

    m_current_level->set_state(value ? LevelState::PAUSED : LevelState::PLAYING);
}

bool Dash::is_paused() const {
    return m_current_level != nullptr && m_current_level->state() == LevelState::PAUSED;
}

bool Dash::has_finished_level() const {
    return m_current_level != nullptr && m_current_level->finished();
}

void Dash::finish_level() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING || player() == nullptr ||
        !player()->alive())
        return;

    m_current_level->set_best_progress(100);
    m_settings->save();
    m_current_level->set_finished(true);
    m_current_level->set_state(LevelState::PAUSED);
}

void Dash::kill_player() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING || player() == nullptr ||
        !player()->alive() || player()->in_god_mode() || has_finished_level()) {
        return;
    }

    std::cout << "[game] starting player death\n";

    m_current_level->set_best_progress(static_cast<int>(m_current_level->progress()));
    m_settings->save();

    player()->freeze();

    // start death animation
    camera().view().zoom = DEFAULT_CAMERA_ZOOM;
    camera().view().rotation = DEFAULT_CAMERA_ROTATION;
    m_current_level->begin_death();
    m_game_ui->show_screen(GameScreen::Death);
}

void Dash::set_music_volume(int volume) {
    m_settings->set_volume(volume);
    m_settings->save();
    if (m_current_level != nullptr) {
        m_current_level->set_music_volume(static_cast<float>(m_settings->volume()) / 100.0f);
    }
}

void Dash::set_godmode(bool enabled) {
    m_settings->set_godmode(enabled);
    m_settings->save();
    if (player() != nullptr) player()->set_god_mode(enabled);
}

void Dash::set_free_mode(bool enabled) {
    m_settings->set_free_mode(enabled);
    m_settings->save();
    if (player() != nullptr) player()->set_free_mode(enabled);
}

void Dash::on_fixed_update(float timestep) {
    Player* current_player = player();
    if (current_player == nullptr) {
        return;
    }

    // update movement and world physics.
    current_player->movement(timestep);
    Game::on_fixed_update(timestep);

    // kill player and show death animation.
    if (current_player->death_requested()) {
        kill_player();
    }

    // focus camera on player.
    if (m_current_level->state() == LevelState::PLAYING) {
        current_player->update_camera(camera());
    }
}

void Dash::on_shutdown() {
    if (m_current_level != nullptr) {
        unload_current_level();
    }

    if (!m_settings->save()) {
        std::cerr << "[game] warning: settings could not be saved\n";
    }

    m_levels.clear();
    m_game_ui = nullptr;
}
