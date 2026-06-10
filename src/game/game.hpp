#pragma once

#include "../ui/ui.hpp"
#include "../level/level.hpp"
#include "object.hpp"

#include <string>
#include <unordered_map>
#include <utility>
#include <filesystem>
#include <vector>

const std::filesystem::path RESOURCES_LOCATION = std::filesystem::path(GetApplicationDirectory()) / "resources";
const std::filesystem::path LEVELS_LOCATION = RESOURCES_LOCATION / "levels";

constexpr float DEFAULT_FIXED_FRAMETIME = 1.0f / 60.0f;

constexpr Vector2 SPRITE_SIZE_HIGH = {128, 128};
constexpr Vector2 SPRITE_SIZE_MEDIUM = {64, 64};

struct Player;
struct Spike;
struct DashLevel;
struct Platform;

struct GameWindow {
    std::string title;
    float width;
    float height;
};

struct Game {
  public:
    explicit Game();
    ~Game();

    GameWindow m_window;
    Player* m_player = nullptr;
    UI m_ui;
    Camera2D m_camera;

    DashLevel* m_current_level = nullptr;

    std::vector<GameObject*> m_objects;
    std::unordered_map<std::string, DashLevel*> m_levels;

    float m_fixed_frametime = DEFAULT_FIXED_FRAMETIME;
    float m_alpha = 0.0f;

    bool m_paused = false;
    bool m_can_pause = true;

    // this will def fuck me later
    void add_game_object(GameObject* obj) {
        obj->id = m_objects.size();
        m_objects.push_back(obj);
    }

    void remove_game_object(GameObject* obj) {
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
    void load_level(std::string_view id, UIMode mode);
    void unload_current_level();
    void load_all_levels();

    // camera related stuff
    void update_camera_focus(GameObject* obj);

  private:
    float m_accumulator = 0.0f;
    bool m_was_paused = false;

    // camera
    GameObject* best_object = nullptr;
    float target_y = 0.0f;

    void handle_pause_state();
    void pause_current_level_music();
    void resume_current_level_music();
    void update_current_level_music();
} inline game;
