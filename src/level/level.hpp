#pragma once

#include "../entity/entity.hpp"

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>

class DashLevel {
public:
    DashLevel() = default;
    ~DashLevel();

    std::vector<std::unique_ptr<Entity>> m_objects;

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

    bool save();
    bool load(std::string_view location);
    bool load_objects(World& world);
    void unload();

    void update();
};
