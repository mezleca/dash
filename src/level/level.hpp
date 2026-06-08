#pragma once

#include "../game/object.hpp"

#include <string>
#include <string_view>
#include <vector>

struct LevelObject {
    ObjectType m_type;
    GameObject* m_obj;
};

struct DashLevel {
    explicit DashLevel(std::string_view location);

    std::vector<LevelObject> objects;

    // metadata
    std::string m_name;
    std::string m_filename;
    std::string m_music_file;

    // progress
    float m_current_progress = 0.0f;

    void save();
    void load(std::string_view location);
    void unload();
};
