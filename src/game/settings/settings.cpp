#include "settings.hpp"
#include "game/dash.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <limits>
#include <string>
#include <utility>

SettingsManager::SettingsManager(Dash& game) : m_game(game) {}

bool SettingsManager::load() {
    m_volume = 100;
    m_godmode = false;
    m_free_mode = false;
    m_loaded_data = nlohmann::json::object();

    std::ifstream file(m_file);
    if (!file) {
        return !fs::exists(m_file);
    }

    auto data = nlohmann::json::parse(file, nullptr, false);
    if (!data.is_object()) {
        return false;
    }

    const auto volume = data.value("volume", nlohmann::json(100));
    if (volume.is_number_integer()) {
        set_volume(static_cast<int>(std::clamp(volume.get<double>(), 0.0, 100.0)));
    }

    const auto godmode = data.find("godmode");
    m_godmode = godmode != data.end() && godmode->is_boolean() && godmode->get<bool>();

    const auto free_mode = data.find("free_mode");
    m_free_mode = free_mode != data.end() && free_mode->is_boolean() && free_mode->get<bool>();

    m_loaded_data = std::move(data);
    return true;
}

void SettingsManager::update_levels_progress() {
    auto& levels = m_game.levels();
    const auto saved_progress = m_loaded_data.find("progress");
    if (saved_progress == m_loaded_data.end() || !saved_progress->is_array()) {
        m_loaded_data.clear();
        return;
    }

    for (const auto& entry : *saved_progress) {
        const auto id = entry.find("id");
        if (id == entry.end() || !id->is_string()) {
            continue;
        }

        auto level = levels.find(id->get_ref<const std::string&>());
        if (level == levels.end()) {
            continue;
        }

        const auto progress = entry.value("progress", nlohmann::json(0));
        const auto attempts = entry.value("attempts", nlohmann::json(0));

        if (progress.is_number_integer()) {
            level->second->set_best_progress(static_cast<int>(std::clamp(progress.get<double>(), 0.0, 100.0)));
        }

        if (attempts.is_number_integer()) {
            level->second->set_attempts(
                static_cast<int>(std::clamp(attempts.get<double>(), 0.0, static_cast<double>(std::numeric_limits<int>::max())))
            );
        }
    }

    m_loaded_data.clear();
}

bool SettingsManager::save() const {
    nlohmann::json data = {
        {"volume", m_volume}, {"godmode", m_godmode}, {"free_mode", m_free_mode}, {"progress", nlohmann::json::array()}
    };

    for (const auto& [id, level] : m_game.levels()) {
        if (level->best_progress() == 0 && level->attempts() == 0) {
            continue;
        }

        data["progress"].push_back({{"id", id}, {"progress", level->best_progress()}, {"attempts", level->attempts()}});
    }

    std::ofstream file(m_file);
    file << data.dump(4) << '\n';
    file.close();
    if (!file) {
        std::cerr << "[settings] warning: unable to write " << m_file << '\n';
        return false;
    }

    return true;
}

void SettingsManager::set_volume(int volume) {
    m_volume = std::clamp(volume, 0, 100);
}
