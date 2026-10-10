#pragma once

#include "game/paths.hpp"

#include <nlohmann/json.hpp>

class Dash;

class SettingsManager {
public:
    explicit SettingsManager(Dash& game);

    bool load();
    void update_levels_progress();
    bool save() const;

    int volume() const {
        return m_volume;
    }

    void set_volume(int volume);

    bool godmode() const {
        return m_godmode;
    }

    void set_godmode(bool enabled) {
        m_godmode = enabled;
    }

    bool free_mode() const {
        return m_free_mode;
    }

    void set_free_mode(bool enabled) {
        m_free_mode = enabled;
    }

private:
    Dash& m_game;
    fs::path m_file = fs::path(GetApplicationDirectory()) / "settings.json";
    int m_volume = 100;
    bool m_godmode = false;
    bool m_free_mode = false;
    nlohmann::json m_loaded_data = nlohmann::json::object();
};
