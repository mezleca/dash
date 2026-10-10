#pragma once

#include "camera.hpp"
#include "object.hpp"
#include "physics/world.hpp"

#include <imgui-ui/surface.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GameWindow {
    std::string title;
    int width;
    int height;
};

class Game {
public:
    explicit Game(GameWindow window);
    virtual ~Game();

    void run();

    // create and store objects
    template <class T, class... Args>
    T& add_object(Args&&... args) {
        auto object = std::make_unique<T>(std::forward<Args>(args)...);
        T& result = *object;
        m_objects.push_back(std::move(object));
        m_render_objects.push_back(&result);
        return result;
    }

    // mark GameObject as pending for removal
    // and remove it on the end of current frame
    void remove_object(GameObject* object);
    void clear_objects();

    const std::vector<std::unique_ptr<GameObject>>& objects() const {
        return m_objects;
    }

    CameraView& camera() {
        return m_camera;
    }

    World& world() {
        return m_world;
    }

    ui::Surface& surface() const {
        return *m_ui;
    }

    const GameWindow& window() const {
        return m_window;
    }

protected:
    virtual void on_initialize() = 0;
    virtual void on_fixed_update(float timestep);
    virtual void on_update(float frametime) = 0;
    virtual void on_shutdown() = 0;
    virtual bool simulation_active() const = 0;

    void reset_simulation_clock();

private:
    void update_simulation_timestep();
    void update_objects(float frametime);
    void render_objects();
    void remove_pending_objects();

    GameWindow m_window;
    std::unique_ptr<ui::Surface> m_ui;
    World m_world;
    CameraView m_camera;
    std::vector<std::unique_ptr<GameObject>> m_objects;
    std::vector<GameObject*> m_render_objects;
    float m_accumulator = 0.0f;
};
