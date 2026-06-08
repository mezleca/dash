#pragma once

#include "../ui/ui.hpp"
#include "object.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <filesystem>
#include <vector>

const std::filesystem::path RESOURCES_LOCATION = std::filesystem::path(GetApplicationDirectory()) / "resources";
const std::filesystem::path LEVELS_LOCATION = RESOURCES_LOCATION / "levels" / "data.json";

constexpr Vector2 SPRITE_SIZE_HIGH = {128, 128};
constexpr Vector2 SPRITE_SIZE_MEDIUM = {64, 64};

struct Player;
struct Spike;
struct Platform;

struct GameWindow {
    std::string title;
    float width;
    float height;
};

struct MinimalLevelData {
    std::filesystem::path location;
    std::string name;
};

inline static void to_json(nlohmann::json& j, const MinimalLevelData& l) {
    j = {
        {"location", l.location},
        {"name", l.name},
    };
}

inline static void from_json(const nlohmann::json& j, MinimalLevelData& l) {
    j.at("location").get_to(l.location);
    j.at("name").get_to(l.name);
}

struct Game {
  public:
    explicit Game();
    ~Game();

    GameWindow window;
    Player* player;
    UI ui;
    Camera2D camera;

    std::vector<GameObject*> m_objects;
    std::vector<MinimalLevelData> levels;

    const float fixed_timestep = 1.0f / 60.0f;
    float alpha = 0.0f;

    bool paused = false;

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

    void load_level(std::string_view location, UIMode mode);
    void load_levels();
    void save_levels();

  private:
    float m_accumulator;
} inline game;
