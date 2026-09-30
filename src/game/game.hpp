#pragma once

#include "../entity/player.hpp"
#include "../level/level.hpp"
#include "object.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

const std::filesystem::path RESOURCES_LOCATION = std::filesystem::path(GetApplicationDirectory()) / "resources";
const std::filesystem::path LEVELS_LOCATION = RESOURCES_LOCATION / "levels";

constexpr float DEFAULT_FIXED_FRAMETIME = 1.0f / 60.0f;

constexpr Vector2 SPRITE_SIZE_HIGH = {128, 128};
constexpr Vector2 SPRITE_SIZE_MEDIUM = {64, 64};

struct Spike;
struct DashLevel;
struct Platform;
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

struct Game {
public:
    explicit Game();
    ~Game();

    void build_ui();

    GameWindow m_window;
    std::unique_ptr<Player> m_player = nullptr;
    ui::Runtime m_runtime;
    std::unique_ptr<ui::Surface> m_ui;
    GameUI* m_game_ui = nullptr;
    Camera2D m_camera;

    // objects / level
    DashLevel* m_current_level = nullptr;
    std::vector<GameObject*> m_objects;
    std::vector<std::unique_ptr<DashLevel>> m_levels;

    // window
    float m_fixed_frametime = DEFAULT_FIXED_FRAMETIME;
    float m_alpha = 0.0f;
    bool m_finished = false;

    // pause
    bool m_paused = false;

    // camera focus
    float m_focus_y = 0.0f;

    LevelState m_level_state = LevelState::LOADING;

    void add_game_object(GameObject* obj) {
        obj->id = static_cast<uint32_t>(m_objects.size());
        m_objects.push_back(obj);
    }

    void remove_game_object(GameObject* obj) {
        if (obj->id >= m_objects.size() || m_objects[obj->id] != obj) {
            auto object_it = std::ranges::find(m_objects, obj);

            if (object_it == m_objects.end()) {
                return;
            }

            obj->id = static_cast<uint32_t>(std::distance(m_objects.begin(), object_it));
        }

        std::swap(m_objects[obj->id], m_objects.back());
        m_objects[obj->id]->id = obj->id;
        m_objects.pop_back();
    }

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
    void update_camera_focus(GameObject* obj);

private:
    float m_accumulator = 0.0f;
    bool m_was_paused = false;

    void handle_pause_state();
    void pause_current_level_music();
    void resume_current_level_music();
    void unload_current_level_music();
    void update_current_level_progress();
    void shutdown();
} inline game;
