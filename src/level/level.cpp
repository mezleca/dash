#include "level.hpp"
#include "../game/game.hpp"
#include "../entity/world/platform.hpp"
#include "../entity/world/spike.hpp"
#include "../entity/world/static.hpp"
#include "../entity/world/finish.hpp"

#include <fstream>
#include <iostream>
#include <memory>

DashLevel::~DashLevel() {
    unload();
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

    return true;
}

bool DashLevel::load_objects() {
    for (const auto& obj_json : m_temp_objects) {
        auto type = static_cast<ObjectType>(json_helper::get_int(obj_json, "type"));
        auto position = json_helper::get_vec2(obj_json, "position");
        auto dimensions = json_helper::get_vec2(obj_json, "dimensions");
        auto visible = json_helper::get_bool(obj_json, "visible");
        auto texture_location = json_helper::get_string(obj_json, "texture");

        // std::cout << "type: " << static_cast<int>(type) << "\n";
        // std::cout << "position: (" << position.x << ", " << position.y << ")\n";
        // std::cout << "dimensions: (" << dimensions.x << ", " << dimensions.y << ")\n";
        // std::cout << "visible: " << visible << "\n";
        // std::cout << "texture_location: " << texture_location << "\n";

        std::unique_ptr<GameObject> obj;

        switch (type) {
            case ObjectType::NONE:
            case ObjectType::BOX: {
                std::cout << "[level] ObjectType::BOX not implemented yet" << "\n";
                break;
            }
            case ObjectType::PLATFORM: {
                obj = std::make_unique<Platform>(dimensions.x, dimensions.y);
                break;
            }
            case ObjectType::SPIKE: {
                auto spike_ammount = json_helper::get_int(obj_json, "spike_ammount");
                obj = std::make_unique<Spike>(spike_ammount);
                break;
            }
            case ObjectType::STATIC_TEXTURE: {
                auto fill_viewport = json_helper::get_bool(obj_json, "fill_viewport");
                obj = std::make_unique<StaticTexture>(texture_location, fill_viewport);
                break;
            }
            default:
                break;
        }

        if (obj == nullptr) {
            std::cout << "[level] unable to find object type: " << static_cast<int>(type) << "\n";
            continue;
        }

        obj->visible = visible;
        obj->position = position;

        // load texture
        // NOTE: texture_location will always be relative to the level folder
        if (!texture_location.empty()) {
            std::filesystem::path full_location = m_file.parent_path() / texture_location;
            obj->load_texture(full_location.c_str());
        }

        // skip spike and empty vectors
        if (type != ObjectType::SPIKE && dimensions.x != 0 && dimensions.y != 0) {
            // std::cout << "updating object (" << static_cast<int>(type) << ") dimension (" << dimensions.x << ", " <<
            // dimensions.y << ")\n";
            obj->dimensions = dimensions;
        }

        // update level end position
        if (type == ObjectType::PLATFORM && obj->position.x + obj->dimensions.x > m_level_end.x) {
            m_level_end = {obj->position.x + obj->dimensions.x, obj->position.y};
        }

        m_objects.push_back(std::move(obj));
    }

    if (m_level_end.x == 0) {
        std::cout << "[level] unable to determine level end position\n";
        unload();
        return false;
    }

    // create fuckass end object
    auto end = std::make_unique<Finish>();
    end->position = {m_level_end.x - end->m_radius, m_level_end.y - end->m_radius};

    m_objects.push_back(std::move(end));
    return true;
}

bool DashLevel::save() {
    std::cout << "[level] saving level at " << m_file << "\n";

    nlohmann::json objects = nlohmann::json::array();

    // serialize each object
    for (const auto& obj : m_objects) {
        objects.push_back(obj->serialize());
    }

    nlohmann::json j = {
        {"name", m_name}, {"music_file", m_music_file}, {"player_start", m_player_start}, {"objects", objects}};

    std::ofstream file(m_file);

    file << j.dump(4);
    file.close();

    return true;
}

void DashLevel::update() {
    m_current_progress = game.m_player->position.x / m_level_end.x * 100;

    if (m_current_progress >= 100.0f) {
        m_current_progress = 100.0f;
        game.finish_level();
    }
}

void DashLevel::unload() {
    m_objects.clear();
    m_level_end = {0, 0};
    m_current_progress = 0.0f;
    m_current_music_progress = 0.0f;
}
