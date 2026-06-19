#pragma once

#include "../game/object.hpp"

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>

struct DashLevel {
    explicit DashLevel() {
    }
    ~DashLevel();

    std::vector<std::unique_ptr<GameObject>> m_objects;

    // metadata
    nlohmann::json m_temp_objects; // will be parsed / initialized on level load
    std::filesystem::path m_file;  // runtime
    std::string m_name;            // from json
    std::string m_music_file;      // from json

    // level start/end
    Vector2 m_player_start; // from json
    Vector2 m_level_end;    // runtime

    // progress
    float m_current_progress = 0.0f;
    float m_current_music_progress = 0.0f;

    // game managed shit
    Music music = {};
    bool m_music_loaded = false;

    bool save();
    bool load(std::string_view location);
    void update();
    bool load_objects();
    void unload();
};

inline static void to_json(nlohmann::json& j, const GameObject* obj) {
    j = {{"type", obj->type},
         {"visible", obj->visible},
         {"texture", obj->texture_location},
         {"position", obj->position},
         {"dimensions", obj->dimensions}};
}
