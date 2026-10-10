#pragma once

#include "core/game.hpp"
#include "game/entity/player.hpp"
#include "game/level/level.hpp"
#include "game/settings/settings.hpp"
#include "editor/camera-controls.hpp"
#include "paths.hpp"

#include <unordered_map>
#include <memory>

constexpr Vector2 SPRITE_SIZE_HIGH = {128, 128};
constexpr Vector2 SPRITE_SIZE_MEDIUM = {64, 64};

class Spike;
class DashLevel;
class Platform;
class GameUI;

class Dash : public Game {
public:
    Dash();

    std::unordered_map<std::string, DashLevel*>& levels() {
        return m_levels;
    }

    const std::unordered_map<std::string, DashLevel*>& levels() const {
        return m_levels;
    }

    Player* player() const {
        return m_current_level != nullptr ? m_current_level->player() : nullptr;
    }

    const DashLevel* current_level() const {
        return m_current_level;
    }

    SettingsManager& settings() {
        return *m_settings;
    }

    void set_paused(bool value);
    bool is_paused() const;
    bool has_finished_level() const;

    LevelState level_state() {
        if (m_current_level == nullptr) return LevelState::INVALID;
        return m_current_level->state();
    }

    bool load_level(DashLevel& level, bool editor = false);
    bool start_level();
    void unload_current_level();
    bool restart_current_level();

    void finish_level();
    void kill_player();

    void set_music_volume(int volume);
    void set_godmode(bool enabled);
    void set_free_mode(bool enabled);

    bool free_mode() const {
        return m_settings->free_mode();
    }

private:
    static constexpr float DEFAULT_CAMERA_ZOOM = 1.2f;
    static constexpr float DEFAULT_CAMERA_ROTATION = 0.0f;

    void build_ui();
    void on_initialize() override;
    void on_fixed_update(float timestep) override;
    void on_update(float frametime) override;
    void on_shutdown() override;
    bool simulation_active() const override;

    std::unique_ptr<SettingsManager> m_settings;
    GameUI* m_game_ui = nullptr;
    EditorCameraControls m_editor_camera_controls;

    DashLevel* m_current_level = nullptr;
    std::unordered_map<std::string, DashLevel*> m_levels;
};
