#include "level.hpp"
#include "../game/game.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

DashLevel::DashLevel(std::string_view location) {
    load(location);
};

void DashLevel::load(std::string_view location) {
    std::cout << "loading level from " << location << "\n";
}

void DashLevel::save() {
    std::filesystem::path final = RESOURCES_LOCATION / "resources" / "levels" / std::string(m_filename + ".json");
    std::cout << "saving level at " << final << "\n";

    nlohmann::json j = {
        {"name", m_name},
        {"filename", m_filename},
    };
}

void DashLevel::unload() {
}
