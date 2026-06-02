#pragma once

#include "../ui/ui.hpp"
#include "../player/player.hpp"

#include <raylib.h>
#include <string>
#include <utility>
#include <vector>

struct GameWindow {
    std::string title;
    float width;
    float height;
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
    UI ui;
    Player player;
    Camera2D camera;

    const float fixed_timestep = 1.0f / 60.0f;
    float alpha = 0.0f;

    // TEMP:
    std::vector<PlaceholderBuild> rects;

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
    float accumulator;
    std::vector<GameObject*> m_objects;
} inline g_game;
