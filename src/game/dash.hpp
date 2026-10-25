#pragma once

#include "core/game.hpp"
#include "game/objects/entities/player.hpp"
#include "game/level/level.hpp"
#include "game/settings/settings.hpp"
#include "editor/editor.hpp"
#include "paths.hpp"

#include <unordered_map>
#include <memory>

constexpr std::string DASH_VERSION = "0.0.1";

constexpr float DEFAULT_CAMERA_ZOOM = 1.2f;
constexpr float DEFAULT_CAMERA_ROTATION = 0.0f;

class DashLevel;
class GameUI;

class Dash : public Game {
public:
    Dash();
    ~Dash() override;

    std::unordered_map<std::string, DashLevel*>& levels() {
        return m_levels;
    }

    const std::unordered_map<std::string, DashLevel*>& levels() const {
        return m_levels;
    }

    Player* player() const {
        return m_current_level != nullptr ? m_current_level->player() : nullptr;
    }

    DashLevel* current_level() {
        return m_current_level;
    }

    const DashLevel* current_level() const {
        return m_current_level;
    }

    LevelEditor* editor() {
        return m_editor.get();
    }

    const LevelEditor* editor() const {
        return m_editor.get();
    }

    SettingsManager& settings() {
        return *m_settings;
    }

    LevelState level_state() {
        if (m_current_level == nullptr) return LevelState::INVALID;
        return m_current_level->state();
    }

    void set_paused(bool value);
    bool is_paused() const;
    bool has_finished_level() const;

    bool load_level(DashLevel& level, bool editor = false, bool force_free_mode = false);
    DashLevel* create_level(const LevelMetadata& metadata);
    bool remove_level(DashLevel& level);
    bool start_level();
    void unload_current_level();
    bool restart_current_level();

    void finish_level();
    void kill_player();
    void return_to_menu();

    void set_music_volume(int volume);
    void set_godmode(bool enabled);
    void set_free_mode(bool enabled);

    bool free_mode() const {
        return m_settings->free_mode();
    }

private:
    CameraView& level_camera() const;
    void update_level_camera(float timestep, bool snap = false);
    void on_initialize() override;
    void on_fixed_update(float timestep) override;
    void on_update(float frametime) override;
    void on_render() override;
    void on_shutdown() override;
    bool simulation_active() const override;

    std::unique_ptr<SettingsManager> m_settings;
    std::unique_ptr<GameUI> m_ui;
    std::unique_ptr<LevelEditor> m_editor;

    DashLevel* m_current_level = nullptr;
    bool m_force_free_mode = false;
    std::unordered_map<std::string, DashLevel*> m_levels;
};
