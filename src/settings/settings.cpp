#include "settings.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

SettingsManager::SettingsManager(std::filesystem::path file) : m_file(std::move(file)) {}

bool SettingsManager::load() {
    std::ifstream file(m_file);
    if (!file.is_open()) {
        return !std::filesystem::exists(m_file);
    }

    try {
        const auto data = nlohmann::json::parse(file);
        const int volume = data.value("volume", 100);
        const bool godmode = data.value("godmode", false);
        std::vector<LevelProgress> progress;

        for (const auto& entry : data.value("progress", nlohmann::json::array())) {
            progress.push_back(
                {entry.at("id").get<std::string>(), std::clamp(entry.at("progress").get<int>(), 0, 100),
                 std::max(entry.at("attempts").get<int>(), 0)}
            );
        }

        m_volume = std::clamp(volume, 0, 100);
        m_godmode = godmode;
        m_progress = std::move(progress);
        return true;
    } catch (const nlohmann::json::exception& error) {
        std::cerr << "[settings] warning: invalid settings file: " << error.what() << '\n';
        return false;
    }
}

bool SettingsManager::save() const {
    nlohmann::json data = {{"volume", m_volume}, {"godmode", m_godmode}, {"progress", nlohmann::json::array()}};

    for (const auto& entry : m_progress) {
        data["progress"].push_back({{"id", entry.id}, {"progress", entry.progress}, {"attempts", entry.attempts}});
    }

    std::ofstream file(m_file);
    if (!file.is_open()) {
        std::cerr << "[settings] warning: unable to open " << m_file << " for writing\n";
        return false;
    }

    file << data.dump(4) << '\n';
    if (!file) {
        std::cerr << "[settings] warning: unable to write " << m_file << '\n';
        return false;
    }

    return true;
}

void SettingsManager::set_volume(int volume) {
    m_volume = std::clamp(volume, 0, 100);
}

const LevelProgress* SettingsManager::progress(std::string_view id) const {
    const auto entry =
        std::find_if(m_progress.begin(), m_progress.end(), [id](const LevelProgress& progress) { return progress.id == id; });
    return entry == m_progress.end() ? nullptr : &*entry;
}

LevelProgress& SettingsManager::get_or_add_progress(std::string_view id) {
    auto entry =
        std::find_if(m_progress.begin(), m_progress.end(), [id](const LevelProgress& progress) { return progress.id == id; });

    if (entry == m_progress.end()) {
        m_progress.push_back({std::string(id)});
        return m_progress.back();
    }

    return *entry;
}

void SettingsManager::set_progress(std::string_view id, int progress) {
    const int value = std::clamp(progress, 0, 100);
    if (const auto* current = this->progress(id); current != nullptr && value <= current->progress) {
        return;
    }

    get_or_add_progress(id).progress = value;
    save();
}

void SettingsManager::update_attempts(std::string_view id) {
    ++get_or_add_progress(id).attempts;
    save();
}
