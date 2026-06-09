#include "level.hpp"
#include "../game/game.hpp"
#include "../entity/world/platform.hpp"
#include "../entity/world/spike.hpp"
#include "nlohmann/json_fwd.hpp"

#include <fstream>
#include <iostream>

bool DashLevel::load(std::string_view location) {
    std::cout << "[level] loading level from " << location << "\n";

    std::ifstream file(location.data());

    if (!file.is_open()) {
        std::cout << "[level] unable to find " << LEVELS_LOCATION << "\n";
        return false;
    }

    nlohmann::json j = nlohmann::json::parse(file);

    m_name = j["name"].get<std::string>();
    m_music_file = j["music_file"].get<std::string>();
    m_player_start = j["player_start"].get<Vector2>();
    m_temp_objects = j.at("objects");

    m_file = std::filesystem::path(location);

    return true;
}

bool DashLevel::load_objects() {
    for (const auto& obj_json : m_temp_objects) {
        auto type = static_cast<ObjectType>(obj_json.at("type").get<int>());
        auto position = obj_json.at("position").get<Vector2>();
        auto dimensions = obj_json.at("dimensions").get<Vector2>();
        auto visible = obj_json.at("visible").get<bool>();
        auto texture_location = obj_json.at("texture").get<std::string>();

        GameObject* obj = nullptr;

        switch (type) {
            case ObjectType::NONE:
            case ObjectType::BOX: {
                std::cout << "[level] ObjectType::BOX not implemented yet" << "\n";
                break;
            }
            case ObjectType::PLATFORM: {
                obj = new Platform(dimensions.x, dimensions.y);
                break;
            }
            case ObjectType::SPIKE: {
                auto spike_ammount = obj_json.at("spike_ammount").get<int>();
                obj = new Spike(spike_ammount);
                break;
            }
            default:
                break;
        }

        if (obj == nullptr) continue;

        obj->visible = visible;
        obj->position = position;

        if (obj->texture_location != "") {
            // NOTE: texture_location will always be relative to the level folder
            std::filesystem::path full_location = m_file.parent_path() / obj->texture_location;
            obj->load_texture(full_location.c_str());
        }

        // skip spike because yes
        if (type != ObjectType::SPIKE) {
            obj->dimensions = dimensions;
        }

        m_objects.push_back(obj);
    }

    m_temp_objects = {};

    return true;
}

bool DashLevel::save() {
    std::filesystem::path file_final = RESOURCES_LOCATION / "resources" / "levels" / m_file;
    std::cout << "[level] saving level at " << file_final << "\n";

    nlohmann::json objects = nlohmann::json::array();

    // serialize each object
    for (const auto* obj : m_objects) {
        objects.push_back(obj->serialize());
    }

    nlohmann::json j = {
        {"name", m_name}, {"music_file", m_music_file}, {"player_start", m_player_start}, {"objects", m_objects}};

    std::ofstream file(file_final);

    file << j.dump(4);
    file.close();

    return true;
}

void DashLevel::unload() {
    for (const auto& object : m_objects) {
        delete object;
    }

    m_objects.clear();
}
