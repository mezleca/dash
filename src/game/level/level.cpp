#include "level.hpp"
#include "game/dash.hpp"
#include "game/entity/player.hpp"
#include "game/entity/world/platform.hpp"
#include "game/entity/world/spike.hpp"
#include "game/entity/world/static.hpp"
#include "game/entity/world/finish.hpp"
#include "game/entity/world/trigger.hpp"
#include "core/math.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

constexpr float DEATH_ANIMATION_DURATION = 1.0f; // in seconds

DashLevel::DashLevel(Dash& game) : m_game(game) {}

DashLevel::~DashLevel() {
    unload();
}

void DashLevel::set_best_progress(int progress) {
    m_best_progress = std::max(m_best_progress, std::clamp(progress, 0, 100));
}

void DashLevel::set_attempts(int attempts) {
    m_attempts = std::max(attempts, 0);
}

void DashLevel::record_attempt() {
    if (m_attempts < std::numeric_limits<int>::max()) {
        ++m_attempts;
    }
}

void DashLevel::set_state(LevelState state) {
    m_state = state;

    if (!m_music_loaded) return;

    if (state == LevelState::PLAYING || state == LevelState::DEATH) {
        ResumeMusicStream(m_music);
    } else {
        PauseMusicStream(m_music);
    }
}

bool DashLevel::update() {
    if (m_music_loaded && m_state == LevelState::PLAYING) {
        UpdateMusicStream(m_music);
    }

    if (m_state == LevelState::EDITING) {
        return false;
    }

    if (m_player == nullptr || m_level_end.x <= 0.0f || m_finished) {
        return false;
    }

    if (m_state == LevelState::DEATH) {
        if (!m_finished_death_animation) update_death_animation();
        return false;
    }

    if (m_state != LevelState::PLAYING) return false;

    m_current_progress = m_player->body().get_position().x / m_level_end.x * 100;
    if (m_current_progress >= 100.0f) {
        m_current_progress = 100.0f;
        return true;
    }

    return false;
}

float DashLevel::death_animation_progress() const {
    return d_easing::out_quad(m_death_elapsed / DEATH_ANIMATION_DURATION);
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

void DashLevel::clear_objects() {
    // release behaviours before destroying entities they may reference.
    m_behaviours.clear();
    m_game.remove_object(m_player);
    m_player = nullptr;
    for (const auto& entry : m_objects) {
        m_game.remove_object(entry.object);
    }
    m_objects.clear();
}

void DashLevel::unload() {
    unload_music();
    clear_objects();
    m_level_end = {0, 0};
    m_current_progress = 0.0f;
    m_finished = false;
    m_state = LevelState::INVALID;
}

void DashLevel::reset() {
    m_behaviours.clear();
    m_current_progress = 0.0f;
    m_finished = false;
    m_finished_death_animation = false;
    m_death_elapsed = 0.0f;
    set_music_pitch(1.0f);
}

bool DashLevel::load(const fs::path& location) {
    std::cout << "[level] loading level from " << location << "\n";

    try {
        load_from_file(location);
    } catch (const std::exception& error) {
        std::cout << "[level] invalid level metadata in " << location << ": " << error.what() << "\n";
        return false;
    }

    m_file = location;

    return true;
}

bool DashLevel::load_objects() {
    clear_objects();
    m_level_end = {};

    m_loading_objects = true;
    try {
        load_from_file(m_file);
    } catch (const std::exception& error) {
        m_loading_objects = false;
        std::cout << "[level] invalid object: " << error.what() << "\n";
        unload();
        return false;
    }

    m_loading_objects = false;
    return true;
}

void DashLevel::serialize(Serializer& serializer) const {
    serializer.field("name", m_name);
    serializer.field("music_file", m_music_file);
    serializer.field("player_start", m_player_start);

    serializer.begin_array("objects");
    for (const auto& object : m_objects) {
        serializer.begin_element();
        serializer.field("type", static_cast<int>(object.type));
        object.object->serialize(serializer);
        serializer.end_element();
    }
    serializer.end_array();
}

void DashLevel::deserialize(Deserializer& deserializer, const fs::path& directory) {
    if (!deserializer.field("name", m_name) || !deserializer.field("music_file", m_music_file) ||
        !deserializer.field("player_start", m_player_start)) {
        throw std::invalid_argument("level metadata is incomplete");
    }

    if (!m_loading_objects) return;

    bool has_finish = false;
    const std::size_t count = deserializer.array_size("objects");
    m_objects.reserve(count + 1);
    for (std::size_t index = 0; index < count; ++index) {
        deserializer.begin_element("objects", index);

        int raw_type = 0;
        if (!deserializer.field("type", raw_type)) {
            throw std::invalid_argument("level object type is missing");
        }

        const auto type = static_cast<ObjectType>(raw_type);
        GameObject* object = nullptr;
        switch (type) {
            case ObjectType::PLATFORM:
                object = &m_game.add_object<Platform>(m_game.world());
                break;
            case ObjectType::SPIKE:
                object = &m_game.add_object<Spike>(m_game.world());
                break;
            case ObjectType::STATIC_TEXTURE:
                object = &m_game.add_object<StaticTexture>(m_game.world());
                break;
            case ObjectType::END:
                object = &m_game.add_object<Finish>(m_game.world());
                has_finish = true;
                break;
            case ObjectType::TRIGGER:
                object = &m_game.add_object<Trigger>(m_game.world(), *this);
                break;
            default:
                std::cout << "[level] unsupported object type: " << raw_type << "\n";
                deserializer.end_element();
                continue;
        }

        // track the object before reading components so a failed load removes it from the game.
        m_objects.push_back({type, object});
        object->deserialize(deserializer, directory);
        auto* body = object->get_component<RigidBody>();

        const Rectangle bounds = body->get_bounding_box();
        if (type == ObjectType::PLATFORM && bounds.x + bounds.width > m_level_end.x) {
            m_level_end = {bounds.x + bounds.width, bounds.y};
        }

        deserializer.end_element();
    }

    if (m_level_end.x == 0.0f) {
        throw std::invalid_argument("unable to determine level end position");
    }

    if (!has_finish) {
        auto& end = m_game.add_object<Finish>(m_game.world());
        auto* body = end.get_component<RigidBody>();
        body->set_position(m_level_end.x - body->get_dimensions().x / 2.0f, m_level_end.y - body->get_dimensions().x / 2.0f);
        m_objects.push_back({ObjectType::END, &end});
    }
}

bool DashLevel::save() {
    std::cout << "[level] saving level at " << m_file << "\n";

    try {
        return save_to_file(m_file);
    } catch (const std::exception& error) {
        std::cout << "[level] failed to save " << m_file << ": " << error.what() << "\n";
        return false;
    }
}

void DashLevel::begin_death() {
    m_death_elapsed = 0.0f;
    m_finished_death_animation = false;
    set_state(LevelState::DEATH);
}

void DashLevel::spawn_player(bool god_mode, bool free_mode) {
    if (m_state == LevelState::EDITING) return;

    if (m_player == nullptr) {
        m_player = &m_game.add_object<Player>(m_game, god_mode);
    } else {
        m_player->reset();
        m_player->set_god_mode(god_mode);
    }

    m_player->set_free_mode(free_mode);
    m_player->body().set_position(m_player_start.x, m_player_start.y);
}

void DashLevel::update_death_animation() {
    m_death_elapsed = std::min(m_death_elapsed + GetFrameTime(), DEATH_ANIMATION_DURATION);
    const float progress = death_animation_progress();
    set_music_pitch(d_math::lerp(1.0f, 0.0f, progress));
    set_music_volume(d_math::lerp(1.0f, 0.0f, progress));

    if (m_death_elapsed >= DEATH_ANIMATION_DURATION && !m_finished_death_animation) {
        m_finished_death_animation = true;
        m_player->kill();
        StopMusicStream(m_music);
    }
}

bool DashLevel::load_music() {
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
    return true;
}

void DashLevel::play_music() {
    if (!m_music_loaded) return;

    StopMusicStream(m_music);
    PlayMusicStream(m_music);
}

void DashLevel::unload_music() {
    if (!m_music_loaded) return;

    StopMusicStream(m_music);
    UnloadMusicStream(m_music);
    m_music = {};
    m_music_loaded = false;
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
