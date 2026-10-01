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
    PAUSED
};

class DashLevel {
public:
    DashLevel() = default;
    ~DashLevel();

    const std::string& name() const {
        return m_name;
    }

    const std::filesystem::path& file() const {
        return m_file;
    }

    LevelState state() const {
        return m_state;
    }

    bool finished() const {
        return m_finished;
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

    void update();
    void update_behaviours(float frametime);
    void add_behaviour(std::unique_ptr<Behaviour> behaviour);

private:
    friend class Game;

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

    // game managed shit
    Music music = {};
    bool m_music_loaded = false;

    bool m_finished = false;
    LevelState m_state = LevelState::INVALID;
};
