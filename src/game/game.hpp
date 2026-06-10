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

    GameWindow window;
    Player* m_player;
    UI ui;
    Camera2D camera;

    DashLevel* m_current_level;

    std::vector<GameObject*> m_objects;
    std::unordered_map<std::string, DashLevel*> m_levels;

    const float fixed_timestep = 1.0f / 60.0f;
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

    void initialize();
    void simulate();
    void render();

    void load_level(std::string_view id, UIMode mode);
    void unload_current_level();
    void load_all_levels();

  private:
    float m_accumulator;
} inline game;
