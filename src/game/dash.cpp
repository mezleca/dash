#include "dash.hpp"
#include "ui/ui.hpp"
#include <iostream>
#include <raylib.h>

Dash::Dash() : Game({"dash", 1280, 720}) {
    // create the default viewport
    auto& viewport = create_viewport(0);
    viewport.camera().view().zoom = DEFAULT_CAMERA_ZOOM;

    m_settings = std::make_unique<SettingsManager>(*this);
}

Dash::~Dash() = default;

DashLevel* Dash::create_level(const LevelMetadata& metadata) {
    auto& level = add_object<DashLevel>(*this);
    if (!level.create(metadata)) {
        remove_object(&level);
        return nullptr;
    }

    m_levels.emplace(level.file().parent_path().filename().string(), &level);
    return &level;
}

bool Dash::remove_level(DashLevel& level) {
    const std::string id = level.file().parent_path().filename().string();
    if (!level.remove()) return false;

    if (m_current_level == &level) {
        unload_current_level();
    }
    m_levels.erase(id);
    remove_object(&level);
    return true;
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
    m_ui = std::make_unique<GameUI>(*this);
}

CameraView& Dash::level_camera() const {
    if (m_current_level != nullptr) {
        if (auto* camera = camera_for(*m_current_level)) {
            return *camera;
        }
    }

    return get_viewport(0)->camera();
}

void Dash::on_update(float frametime) {
    if (!m_ui->process_events()) {
        exit();
        return;
    }

    // the gameplay viewport fills the window outside the editor.
    const bool editing = m_current_level != nullptr && m_current_level->state() == LevelState::EDITING;
    if (!editing) {
        if (auto* viewport = get_viewport(0)) {
            viewport->set_position({});
            viewport->set_size({static_cast<float>(window().width), static_cast<float>(window().height)});
        }
    }

    // early return if we're not in a level
    if (m_current_level == nullptr || m_current_level->state() == LevelState::INVALID) {
        return;
    }

    m_ui->update_editor();
    m_current_level->update();

    // update level behaviours and remove completed ones before entity components run.
    if (m_current_level->state() != LevelState::DEATH) {
        m_current_level->update_behaviours(frametime);
    }
}

void Dash::on_render() {
    m_ui->draw(GetFrameTime());
}

bool Dash::simulation_active() const {
    return m_current_level != nullptr && m_current_level->state() == LevelState::PLAYING;
}

bool Dash::load_level(DashLevel& level, bool editor, bool force_free_mode) {
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

    auto& camera = level_camera();
    camera.view().zoom = DEFAULT_CAMERA_ZOOM;
    camera.view().rotation = DEFAULT_CAMERA_ROTATION;
    m_editor.reset();

    if (editor) {
        camera.set_target(level.player_start());
    } else {
        level.spawn_player(m_settings->godmode(), force_free_mode || free_mode());
        level.player()->update_camera(camera, level, true);
        level.set_music_volume(static_cast<float>(m_settings->volume()) / 100.0f);

        if (!level.load_music()) {
            level.unload();
            camera.set_target({});
            return false;
        }
    }

    m_current_level = &level;
    m_force_free_mode = force_free_mode && !editor;
    std::cout << "loaded " << level.file() << " successfully\n";

    return true;
}

bool Dash::start_level() {
    if (m_current_level == nullptr) {
        std::cout << "failed to start current level (not loaded)\n";
        return false;
    }

    auto& camera = level_camera();
    camera.view().zoom = DEFAULT_CAMERA_ZOOM;
    camera.view().rotation = DEFAULT_CAMERA_ROTATION;
    m_editor.reset();
    reset_simulation_clock();

    m_current_level->reset();

    // configure editor.
    if (m_current_level->state() == LevelState::EDITING) {
        // TOFIX: use latest editing position instead of player start.
        camera.set_target(m_current_level->player_start());

        if (auto* viewport = get_viewport(0)) {
            m_editor = std::make_unique<LevelEditor>(*viewport, *m_current_level);
        }

        return true;
    }

    // configure player and focus the camera.
    m_current_level->spawn_player(m_settings->godmode(), m_force_free_mode || free_mode());
    player()->update_camera(camera, *m_current_level, true);

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

    auto& camera = level_camera();
    camera.view().zoom = DEFAULT_CAMERA_ZOOM;
    camera.view().rotation = DEFAULT_CAMERA_ROTATION;
    camera.set_target({});

    m_editor.reset();

    reset_simulation_clock();
    m_current_level = nullptr;
    m_force_free_mode = false;
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

void Dash::return_to_menu() {
    if (m_current_level != nullptr) {
        unload_current_level();
    }

    m_ui->show_screen(GameScreen::Menu);
}

void Dash::finish_level() {
    if (m_current_level == nullptr || m_current_level->state() != LevelState::PLAYING || player() == nullptr ||
        !player()->alive())
        return;

    m_current_level->set_best_progress(100);
    m_settings->save();
    m_current_level->set_finished(true);
    m_current_level->set_state(LevelState::PAUSED);
    m_ui->show_screen(GameScreen::Finished);
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
    auto& camera = level_camera();
    camera.view().zoom = DEFAULT_CAMERA_ZOOM;
    camera.view().rotation = DEFAULT_CAMERA_ROTATION;
    m_current_level->begin_death();
    m_ui->show_screen(GameScreen::Death);
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
    if (player() != nullptr) player()->set_free_mode(m_force_free_mode || enabled);
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
        if (auto* camera = camera_for(*current_player)) {
            current_player->update_camera(*camera, *m_current_level);
        }
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
    m_ui.reset();
}
