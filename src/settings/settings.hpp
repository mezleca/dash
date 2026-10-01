#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

struct LevelProgress {
    std::string id;
    int progress = 0;
    int attempts = 0;
};

class SettingsManager {
public:
    explicit SettingsManager(std::filesystem::path file);

    bool load();
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

    const LevelProgress* progress(std::string_view id) const;
    void set_progress(std::string_view id, int progress);
    void update_attempts(std::string_view id);

private:
    LevelProgress& get_or_add_progress(std::string_view id);

    std::filesystem::path m_file;
    std::vector<LevelProgress> m_progress;
    int m_volume = 100;
    bool m_godmode = false;
};
