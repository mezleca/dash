#include "level.hpp"
#include "../entity/player.hpp"
#include "../game/game.hpp"
#include "../entity/world/platform.hpp"
#include "../entity/world/spike.hpp"
#include "../entity/world/static.hpp"
#include "../entity/world/finish.hpp"
#include "../entity/world/trigger.hpp"
#include "../utils/math.hpp"

#include <imgui-ui/transition.hpp>

#include <raymath.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

constexpr float DEATH_ANIMATION_DURATION = 1.0f; // in seconds
constexpr float DEATH_ZOOM_MULTIPLIER = 1.12f;
constexpr float DEATH_ROTATION_OFFSET = 5.0f;

DashLevel::DashLevel() = default;

DashLevel::~DashLevel() {
    unload();
}

void DashLevel::update_edit_mode() {
    float wheel_y = GetMouseWheelMoveV().y;
    if (wheel_y != 0.0f) {
        float new_zoom = Clamp(game.camera().zoom() + wheel_y * 0.25f, 0.1f, 10.0f);
        game.camera().set_zoom(new_zoom);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        m_in_drag_mode = true;
        m_drag_start_pos = game.camera().transform().target;
        m_drag_start_mouse_pos = GetMousePosition();
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) m_in_drag_mode = false;
    if (!m_in_drag_mode) return;

    Vector2 mouse_delta = Vector2Subtract(m_drag_start_mouse_pos, GetMousePosition());
    Vector2 world_delta = Vector2Scale(mouse_delta, 1.0f / game.camera().zoom());
    game.camera().set_target(Vector2Add(m_drag_start_pos, world_delta));
}

void DashLevel::update() {
    update_music();

    // update editor move, zoom etc... and early return
    if (m_state == LevelState::EDITING) {
        update_edit_mode();
        return;
    }

    // early return if we didn't finished the level yet
    if (m_player == nullptr || m_level_end.x <= 0.0f || m_finished) {
        return;
    }

    // update death animation
    if (m_state == LevelState::DEATH) {
        if (!m_finished_death_animation) update_death_animation();
        return;
    }

    if (m_state != LevelState::PLAYING) return;

    m_current_progress = m_player->get_position().x / m_level_end.x * 100;
    if (m_current_progress >= 100.0f) {
        m_current_progress = 100.0f;
        game.finish_level();
    }
}

void DashLevel::add_behaviour(std::unique_ptr<Behaviour> behaviour) {
    if (behaviour != nullptr) {
        m_behaviours.push_back(std::move(behaviour));
    }
}

void DashLevel::update_behaviours(float frametime) {
    // behaviours added during an update start on the next frame.
    const size_t count = m_behaviours.size();

    for (size_t i = 0; i < count; ++i) {
        if (!m_behaviours[i]->is_finished()) {
            m_behaviours[i]->update(frametime);
        }
    }

    // remove completed behaviours after every callback returns.
    std::erase_if(m_behaviours, [](const auto& behaviour) { return behaviour->is_finished(); });
}

void DashLevel::unload() {
    unload_music();
    m_behaviours.clear();
    m_player.reset();
    m_objects.clear();
    m_level_end = {0, 0};
    m_current_progress = 0.0f;
    m_current_music_progress = 0.0f;
    m_finished = false;
    m_state = LevelState::INVALID;
}

void DashLevel::reset() {
    m_behaviours.clear();
    m_current_progress = 0.0f;
    m_current_music_progress = 0.0f;
    m_finished = false;
    m_finished_death_animation = false;
    m_death_elapsed = 0.0f;
    set_music_pitch(1.0f);
}

bool DashLevel::load(std::string_view location) {
    std::cout << "[level] loading level from " << location << "\n";

    std::ifstream file(location.data());
    if (!file.is_open()) {
        std::cout << "[level] unable to find " << LEVELS_LOCATION << "\n";
        return false;
    }

    nlohmann::json j;

    try {
        j = nlohmann::json::parse(file);
    } catch (const nlohmann::json::exception& error) {
        std::cout << "[level] failed to parse " << location << ": " << error.what() << "\n";
        return false;
    }

    try {
        m_name = j.at("name").get<std::string>();
        m_music_file = j.at("music_file").get<std::string>();
        m_player_start = j.at("player_start").get<Vector2>();
        m_temp_objects = j.at("objects");
    } catch (const nlohmann::json::exception& error) {
        std::cout << "[level] invalid level metadata in " << location << ": " << error.what() << "\n";
        return false;
    }

    m_level_end = {0, 0};

    m_file = std::filesystem::path(location);
    m_fully_loaded = false;

    return true;
}

bool DashLevel::load_objects(World& world) {
    // release behaviours before destroying entities they may reference.
    m_behaviours.clear();
    m_objects.clear();
    m_level_end = {};

    bool has_finish = false;

    try {
        for (const auto& data : m_temp_objects) {
            const auto type = data.at("type").get<ObjectType>();
            std::unique_ptr<Entity> object;

            switch (type) {
                case ObjectType::PLATFORM:
                    object = std::make_unique<Platform>(world);
                    break;
                case ObjectType::SPIKE:
                    object = std::make_unique<Spike>(world);
                    break;
                case ObjectType::STATIC_TEXTURE:
                    object = std::make_unique<StaticTexture>(world);
                    break;
                case ObjectType::END:
                    object = std::make_unique<Finish>(world);
                    has_finish = true;
                    break;
                case ObjectType::TRIGGER:
                    object = std::make_unique<Trigger>(world, *this);
                    break;
                default:
                    std::cout << "[level] unsupported object type: " << static_cast<int>(type) << "\n";
                    continue;
            }

            // shape and position must be loaded before bounds determine the level end.
            object->deserialize(data, m_file.parent_path());

            const Rectangle bounds = object->get_bounding_box();

            if (type == ObjectType::PLATFORM && bounds.x + bounds.width > m_level_end.x) {
                m_level_end = {bounds.x + bounds.width, bounds.y};
            }

            m_objects.push_back(std::move(object));
        }
    } catch (const std::exception& error) {
        std::cout << "[level] invalid object: " << error.what() << "\n";
        unload();
        return false;
    }

    if (m_level_end.x == 0.0f) {
        std::cout << "[level] unable to determine level end position\n";
        unload();
        return false;
    }

    if (!has_finish) {
        auto end = std::make_unique<Finish>(world);
        end->set_position(m_level_end.x - end->get_dimensions().x / 2.0f, m_level_end.y - end->get_dimensions().x / 2.0f);
        m_objects.push_back(std::move(end));
    }

    m_fully_loaded = true;
    return true;
}

bool DashLevel::save() {
    std::cout << "[level] saving level at " << m_file << "\n";

    nlohmann::json objects = nlohmann::json::array();
    for (const auto& obj : m_objects) {
        objects.push_back(obj->serialize());
    }

    nlohmann::json j = {{"name", m_name}, {"music_file", m_music_file}, {"player_start", m_player_start}, {"objects", objects}};
    std::ofstream file(m_file);

    if (!file.is_open()) {
        return false;
    }

    file << j.dump() << '\n';
    file.close();

    return !file.fail();
}

void DashLevel::begin_death() {
    m_death_elapsed = 0.0f;
    m_finished_death_animation = false;
    game.camera().reset();
    set_state(LevelState::DEATH);
}

void DashLevel::spawn_player(World& world, bool god_mode, bool free_mode) {
    if (m_state == LevelState::EDITING) return;

    if (m_player == nullptr) {
        m_player = std::make_unique<Player>(world, god_mode);
    } else {
        m_player->reset();
        m_player->set_god_mode(god_mode);
    }

    m_player->set_free_mode(free_mode);
    m_player->set_position(m_player_start.x, m_player_start.y);
}

void DashLevel::update_death_animation() {
    m_death_elapsed = std::min(m_death_elapsed + GetFrameTime(), DEATH_ANIMATION_DURATION);
    const float progress = ui::easing::out_quad(m_death_elapsed / DEATH_ANIMATION_DURATION);

    auto& camera = game.camera();
    camera.set_zoom(d_math::lerp(DEFAULT_CAMERA_ZOOM, DEFAULT_CAMERA_ZOOM * DEATH_ZOOM_MULTIPLIER, progress));
    camera.set_rotation(d_math::lerp(DEFAULT_CAMERA_ROTATION, DEFAULT_CAMERA_ROTATION + DEATH_ROTATION_OFFSET, progress));
    set_music_pitch(d_math::lerp(1.0f, 0.0f, progress));
    set_music_volume(d_math::lerp(1.0f, 0.0f, progress));

    if (m_death_elapsed >= DEATH_ANIMATION_DURATION && !m_finished_death_animation) {
        m_finished_death_animation = true;
        m_player->kill();
        StopMusicStream(m_music);
    }
}

bool DashLevel::load_music(bool autoplay) {
    unload_music();

    const auto location = m_file.parent_path() / m_music_file;
    m_music = LoadMusicStream(location.c_str());
    m_music_loaded = IsMusicValid(m_music);
    if (!m_music_loaded) {
        std::cout << "[level] failed to load music from " << location << '\n';
        return false;
    }

    SetMusicPitch(m_music, m_music_pitch);
    SetMusicVolume(m_music, m_music_volume);
    SetMusicPan(m_music, m_music_pan);
    if (autoplay) PlayMusicStream(m_music);
    m_current_music_progress = 0.0f;
    return true;
}

void DashLevel::play_music() {
    if (!m_music_loaded) return;

    StopMusicStream(m_music);
    set_music_progress(0.0f);
    PlayMusicStream(m_music);
}

void DashLevel::unload_music() {
    if (!m_music_loaded) return;

    StopMusicStream(m_music);
    UnloadMusicStream(m_music);
    m_music = {};
    m_music_loaded = false;
    m_current_music_progress = 0.0f;
}

void DashLevel::update_music() {
    if (!m_music_loaded || m_state != LevelState::PLAYING) return;

    UpdateMusicStream(m_music);
    m_current_music_progress = GetMusicTimePlayed(m_music);
}

void DashLevel::set_music_pitch(float pitch) {
    if (pitch <= 0.0f) return;

    m_music_pitch = pitch;
    if (m_music_loaded) SetMusicPitch(m_music, pitch);
}

void DashLevel::set_music_volume(float volume) {
    m_music_volume = std::clamp(volume, 0.0f, 1.0f);
    if (m_music_loaded) SetMusicVolume(m_music, m_music_volume);
}

void DashLevel::set_music_pan(float pan) {
    m_music_pan = std::clamp(pan, 0.0f, 1.0f);
    if (m_music_loaded) SetMusicPan(m_music, m_music_pan);
}

void DashLevel::set_music_progress(float seconds) {
    m_current_music_progress = std::max(0.0f, seconds);
    if (m_music_loaded) SeekMusicStream(m_music, m_current_music_progress);
}
