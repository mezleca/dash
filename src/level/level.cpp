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

bool DashLevel::load_objects(World& world) {
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
                case ObjectType::SPIKE: {
                    const int amount = data.at("spike_ammount").get<int>();

                    if (amount <= 0) {
                        std::cout << "[level] invalid spike amount\n";
                        unload();
                        return false;
                    }

                    object = std::make_unique<Spike>(world, amount);
                    break;
                }
                case ObjectType::STATIC_TEXTURE:
                    object = std::make_unique<StaticTexture>(world, data.value("fill_viewport", false));
                    break;
                case ObjectType::END:
                    object = std::make_unique<Finish>(world);
                    has_finish = true;
                    break;
                default:
                    std::cout << "[level] unsupported object type: " << static_cast<int>(type) << "\n";
                    continue;
            }

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
        end->set_position(m_level_end.x - end->m_radius, m_level_end.y - end->m_radius);
        m_objects.push_back(std::move(end));
    }

    return true;
}

bool DashLevel::save() {
    std::cout << "[level] saving level at " << m_file << "\n";

    nlohmann::json objects = nlohmann::json::array();

    // serialize each object
    for (const auto& obj : m_objects) {
        objects.push_back(obj->serialize());
    }

    nlohmann::json j = {{"name", m_name}, {"music_file", m_music_file}, {"player_start", m_player_start}, {"objects", objects}};

    std::ofstream file(m_file);

    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    file.close();

    return !file.fail();
}

void DashLevel::update() {
    m_current_progress = game.m_player->get_position().x / m_level_end.x * 100;

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
