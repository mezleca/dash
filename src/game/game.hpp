#pragma once

#include "../entity/player.hpp"
#include "../level/level.hpp"
#include "../settings/settings.hpp"
#include "camera.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <iostream>
#include <vector>

const std::filesystem::path RESOURCES_LOCATION = std::filesystem::path(GetApplicationDirectory()) / "resources";
const std::filesystem::path LEVELS_LOCATION = RESOURCES_LOCATION / "levels";

constexpr float DEFAULT_FIXED_FRAMETIME = 1.0f / 60.0f;

constexpr Vector2 SPRITE_SIZE_HIGH = {128, 128};
constexpr Vector2 SPRITE_SIZE_MEDIUM = {64, 64};

class Spike;
class DashLevel;
class Platform;
class GameUI;

struct GameWindow {
    std::string title;
    int width;
    int height;
};

class Game {
public:
    explicit Game();
    ~Game() = default;

    void initialize();

    ui::Surface& surface() const {
        return *m_ui;
    }

    const std::vector<std::unique_ptr<DashLevel>>& levels() const {
        return m_levels;
    }

    Player* player() const {
        return m_current_level != nullptr ? m_current_level->player() : nullptr;
    }

    const DashLevel* current_level() const {
        return m_current_level;
    }

    GameCamera& camera() {
        return m_camera;
    }

    SettingsManager& settings() {
        return m_settings;
    }

    float fixed_frametime() const {
        return m_fixed_frametime;
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
    void finish_level_loading(bool loaded);

    void pause_level();
    void resume_level();
    void return_to_menu();

    void finish_level();
    void kill_player();

    void set_music_volume(int volume);
    void set_godmode(bool enabled);
    void set_free_mode(bool enabled);

    bool free_mode() const {
        return m_free_mode;
    }

    bool pointer_over_ui() const {
        return m_pointer_over_ui;
    }

private:
    void build_ui();
    void load_all_levels();

    void update_simulation_timestep();
    void simulate();
    void render();

    void handle_pause_state();

    void shutdown();

    World m_world;
    SettingsManager m_settings{std::filesystem::path(GetApplicationDirectory()) / "settings.json"};
    GameWindow m_window;
    std::unique_ptr<ui::Surface> m_ui;
    GameUI* m_game_ui = nullptr;
    GameCamera m_camera;

    DashLevel* m_current_level = nullptr;
    std::vector<std::unique_ptr<DashLevel>> m_levels;
    std::vector<Entity*> m_render_objects;

    float m_fixed_frametime = DEFAULT_FIXED_FRAMETIME;
    float m_alpha = 0.0f;
    float m_accumulator = 0.0f;
    bool m_was_paused = false;
    bool m_free_mode = false;
    bool m_pointer_over_ui = false;
} inline game;
