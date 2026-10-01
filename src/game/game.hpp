#pragma once

#include "../entity/player.hpp"
#include "../level/level.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <filesystem>
#include <memory>
#include <string>
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

enum class LevelState : uint8_t {
    LOADING = 0,
    PLAYING,
    PAUSED,
    DEATH,
    FINISHED
};

class Game {
public:
    explicit Game();
    ~Game() = default;

    void build_ui();

    World m_world;
    GameWindow m_window;
    std::unique_ptr<Player> m_player = nullptr;
    ui::Runtime m_runtime;
    std::unique_ptr<ui::Surface> m_ui;
    GameUI* m_game_ui = nullptr;
    Camera2D m_camera;

    // objects / level
    DashLevel* m_current_level = nullptr;
    std::vector<std::unique_ptr<DashLevel>> m_levels;

    // window
    float m_fixed_frametime = DEFAULT_FIXED_FRAMETIME;
    float m_alpha = 0.0f;

    // camera focus
    float m_focus_y = 0.0f;

    bool m_finished = false;

    // pause
    bool m_paused = false;

    LevelState m_level_state = LevelState::LOADING;

    // core
    void initialize();
    void update_simulation_timestep();
    void simulate();
    void render();

    // level related stuff
    bool load_level(DashLevel& level);
    bool start_level();
    void unload_current_level();
    bool restart_current_level();
    void finish_level_loading(bool loaded);
    void pause_level();
    void resume_level();
    void return_to_menu();
    void load_all_levels();
    void finish_level();
    void kill_player();

    // camera related stuff
    void update_camera_focus(Entity* obj);

private:
    std::vector<GameObject*> m_render_objects;
    float m_accumulator = 0.0f;
    bool m_was_paused = false;

    void handle_pause_state();
    void pause_current_level_music();
    void resume_current_level_music();
    void unload_current_level_music();
    void update_current_level_progress();
    void shutdown();
} inline game;
