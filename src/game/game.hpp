#pragma once

#include "../ui/ui.hpp"

#include <raylib.h>
#include <string>
#include <cstdint>
#include <utility>
#include <vector>

constexpr Vector2 SPRITE_SIZE_HIGH = { 128, 128 };
constexpr Vector2 SPRITE_SIZE_MEDIUM = { 64, 64 };

struct Player;
struct Spike;
struct Platform;

struct GameWindow {
    std::string title;
    float width;
    float height;
};

struct GameObject {
    explicit GameObject();
    ~GameObject();

    std::string name;

    Texture2D texture;
    Vector2 position;
    Vector2 dimensions;
    uint32_t id = 0;

    void load_texture(const char* location);
    virtual void render() = 0;
};

struct PlaceholderBuild {
    Rectangle rect;
    Color color;
};

struct Game {
  public:
    explicit Game();
    ~Game();

    GameWindow window;
    Player* player;
    UI ui;
    Camera2D camera;

    std::vector<GameObject*> m_objects;

    // TEMP STUFF ----
    std::vector<PlaceholderBuild> rects;
    Music test_level_music;
    Sound explosion;
    // ------

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

  private:
    float m_accumulator;
} inline game;
