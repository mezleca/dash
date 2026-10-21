#pragma once

#include "object.hpp"
#include "viewport/viewport.hpp"
#include "physics/world.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

static constexpr std::uint32_t MAX_VIEWPORTS = 32;

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
    void exit();

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
    // and remove it on the end of the current frame
    void remove_object(GameObject* object);
    void clear_objects();

    Viewport& create_viewport(ViewportId id);
    Viewport* get_viewport(size_t index) const;
    size_t viewport_count() const {
        return m_viewports.size();
    }

    CameraView* camera_for(const GameObject& object) const;

    const std::vector<std::unique_ptr<GameObject>>& objects() const {
        return m_objects;
    }

    World& world() {
        return m_world;
    }

    const GameWindow& window() const {
        return m_window;
    }

protected:
    virtual void on_initialize() = 0;
    virtual void on_fixed_update(float timestep);
    virtual void on_update(float frametime) = 0;
    virtual void on_render() {}
    virtual void on_shutdown() = 0;
    virtual bool simulation_active() const = 0;

    void reset_simulation_clock();

private:
    void update_simulation_timestep();
    void update();
    void render();
    void render_viewport(Viewport& viewport);
    void remove_pending_objects();

    GameWindow m_window;
    World m_world;

    std::vector<std::unique_ptr<Viewport>> m_viewports;
    std::vector<std::unique_ptr<GameObject>> m_objects;
    std::vector<GameObject*> m_render_objects;

    float m_accumulator = 0.0f;
    bool m_done = false;
};
