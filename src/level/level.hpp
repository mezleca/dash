#pragma once

#include "../entity/entity.hpp"
#include "behaviour.hpp"

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>

enum class LevelState : int8_t {
    INVALID = -1, // corrupted, not loaded, whatever
    LOADING = 0,
    PLAYING,
    PAUSED,
    DEATH
};

class DashLevel {
public:
    DashLevel() = default;
    ~DashLevel();
    DashLevel(const DashLevel&) = delete;
    DashLevel& operator=(const DashLevel&) = delete;
    DashLevel(DashLevel&&) = delete;
    DashLevel& operator=(DashLevel&&) = delete;

    const std::string& name() const {
        return m_name;
    }

    const std::filesystem::path& file() const {
        return m_file;
    }

    void set_state(LevelState state) {
        m_state = state;

        if (!m_music_loaded) {
            return;
        }

        if (state == LevelState::PLAYING || state == LevelState::DEATH) {
            ResumeMusicStream(m_music);
        } else {
            PauseMusicStream(m_music);
        }
    }

    LevelState state() const {
        return m_state;
    }

    bool finished() const {
        return m_finished;
    }

    void set_finished(bool value) {
        m_finished = value;
    }

    Vector2 player_start() const {
        return m_player_start;
    }

    float progress() const {
        return m_current_progress;
    }

    const std::vector<std::unique_ptr<Entity>>& objects() const {
        return m_objects;
    }

    bool save();
    bool load(std::string_view location);

    bool load_objects(World& world);
    void unload();
    void reset();
    void begin_death();

    const Music& music() const {
        return m_music;
    }

    bool music_loaded() const {
        return m_music_loaded;
    }

    float music_pitch() const {
        return m_music_pitch;
    }

    float music_volume() const {
        return m_music_volume;
    }

    float music_pan() const {
        return m_music_pan;
    }

    float music_progress() const {
        return m_current_music_progress;
    }

    bool load_music();
    void unload_music();
    void set_music_pitch(float pitch);
    void set_music_volume(float volume);
    void set_music_pan(float pan);
    void set_music_progress(float seconds);

    void update();
    void update_behaviours(float frametime);
    void add_behaviour(std::unique_ptr<Behaviour> behaviour);

private:
    void update_music();
    void update_death_animation();

    std::vector<std::unique_ptr<Entity>> m_objects;
    std::vector<std::unique_ptr<Behaviour>> m_behaviours;

    // metadata
    nlohmann::json m_temp_objects; // will be parsed / initialized on level load
    std::filesystem::path m_file;  // runtime
    std::string m_name;            // from json
    std::string m_music_file;      // from json

    // level start/end
    Vector2 m_player_start = {}; // from json
    Vector2 m_level_end = {};    // runtime

    // progress
    float m_current_progress = 0.0f;
    float m_current_music_progress = 0.0f;

    Music m_music = {};
    float m_music_pitch = 1.0f;
    float m_music_volume = 1.0f;
    float m_music_pan = 0.0f;
    bool m_music_loaded = false;

    bool m_finished = false;

    bool m_finished_death_animation = false;
    float m_death_elapsed = 0.0f;

    LevelState m_state = LevelState::INVALID;
};
