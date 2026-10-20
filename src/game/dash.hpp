#pragma once

#include "core/game.hpp"
#include "game/entity/player.hpp"
#include "game/level/level.hpp"
#include "game/settings/settings.hpp"
#include "editor/editor.hpp"
#include "paths.hpp"

#include <unordered_map>
#include <memory>

constexpr Vector2 SPRITE_SIZE_HIGH = {128, 128};
constexpr Vector2 SPRITE_SIZE_MEDIUM = {64, 64};
constexpr float DEFAULT_CAMERA_ZOOM = 1.2f;
constexpr float DEFAULT_CAMERA_ROTATION = 0.0f;

class Spike;
class DashLevel;
class Platform;
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

    bool load_level(DashLevel& level, bool editor = false);
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

    ui::Surface& surface() const {
        return *m_ui;
    }

    bool is_debugger_open() const {
        return m_ui->debugger()->is_open();
    }

private:
    void build_ui();
    CameraView& level_camera() const;
    void on_initialize() override;
    void on_fixed_update(float timestep) override;
    void on_update(float frametime) override;
    void on_render() override;
    void on_shutdown() override;
    bool simulation_active() const override;

    std::unique_ptr<ui::Runtime> m_runtime;
    std::unique_ptr<ui::Surface> m_ui;

    std::unique_ptr<SettingsManager> m_settings;
    GameUI* m_game_ui = nullptr;
    std::unique_ptr<LevelEditor> m_editor;

    DashLevel* m_current_level = nullptr;
    std::unordered_map<std::string, DashLevel*> m_levels;
};
