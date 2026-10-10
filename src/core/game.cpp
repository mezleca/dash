#include "game.hpp"
#include "components/rigid-body.hpp"

#include <imgui-ui/backends/opengl/texture-loader.hpp>
#include <imgui-ui/backends/raylib/backend.hpp>
#include <imgui-ui/runtime.hpp>

#include <raylib.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <utility>

constexpr float FIXED_FRAMETIME = 1.0f / 60.0f;

Game::Game(GameWindow window) : m_window(std::move(window)) {}

Game::~Game() {
    clear_objects();
    remove_pending_objects();
}

void Game::on_fixed_update(float timestep) {
    m_world.step(timestep);
}

void Game::remove_object(GameObject* object) {
    auto entry = std::find_if(m_objects.begin(), m_objects.end(), [object](const auto& owned) { return owned.get() == object; });
    if (entry == m_objects.end()) return;

    (*entry)->m_removed = true;
}

void Game::clear_objects() {
    for (const auto& object : m_objects)
        object->m_removed = true;
}

void Game::remove_pending_objects() {
    auto entry = std::find_if(m_objects.begin(), m_objects.end(), [](const auto& object) { return object->m_removed; });
    while (entry != m_objects.end()) {
        // detach before destruction because an object's destructor can mark other objects for removal.
        auto removed = std::move(*entry);
        m_objects.erase(entry);
        std::erase(m_render_objects, removed.get());
        removed.reset();
        entry = std::find_if(m_objects.begin(), m_objects.end(), [](const auto& object) { return object->m_removed; });
    }
}

void Game::update_objects(float frametime) {
    const std::size_t count = m_objects.size();

    for (std::size_t index = 0; index < count; ++index) {
        GameObject* object = m_objects[index].get();
        if (!object->m_removed) {
            object->update(frametime);
        }
    }
}

void Game::render_objects() {
    m_camera.center_viewport({static_cast<float>(m_window.width), static_cast<float>(m_window.height)});

    const auto compare = [](const GameObject* a, const GameObject* b) { return a->z_order() < b->z_order(); };

    // order GameObject by its index before rendering
    if (!std::is_sorted(m_render_objects.begin(), m_render_objects.end(), compare)) {
        std::stable_sort(m_render_objects.begin(), m_render_objects.end(), compare);
    }

    BeginMode2D(m_camera.view());
    {
        const std::size_t count = m_render_objects.size();
        for (std::size_t index = 0; index < count; ++index) {
            // skip pending and not visible game objects
            const GameObject* object = m_render_objects[index];
            if (object->m_removed || !object->visible) {
                continue;
            }

            const auto* body = object->get_component<RigidBody>();
            const Rectangle bounds = body != nullptr ? body->get_bounding_box() : Rectangle{};
            object->render(bounds, m_camera.view());
        }
    }
    EndMode2D();
}

void Game::run() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(0);

    ui::Runtime runtime({.texture_loader = std::make_unique<ui::OpenGLTextureLoader>()});
    ui::SurfaceConfig config;
    config.backend = std::make_unique<ui::RaylibBackend>();
    config.enable_debugger = true;
    m_ui = std::make_unique<ui::Surface>(runtime, std::move(config));

    on_initialize();

    while (!m_ui->is_done()) {
        update_simulation_timestep();
        m_ui->process_events();

        if (IsWindowResized()) {
            m_window.width = GetScreenWidth();
            m_window.height = GetScreenHeight();
        }

        const float frametime = GetFrameTime();
        // game logic changes levels and runs behaviours before their components update.
        on_update(frametime);
        update_objects(frametime);

        m_ui->begin_frame();
        render_objects();
        m_ui->update(frametime);
        m_ui->draw();
        m_ui->end_frame();
        remove_pending_objects();
    }

    on_shutdown();
    clear_objects();
    remove_pending_objects();
    m_ui.reset();
    CloseAudioDevice();
    CloseWindow();
}

void Game::update_simulation_timestep() {
    if (!simulation_active()) {
        reset_simulation_clock();
        return;
    }

    m_accumulator += GetFrameTime();

    while (m_accumulator >= FIXED_FRAMETIME) {
        m_accumulator -= FIXED_FRAMETIME;
        on_fixed_update(FIXED_FRAMETIME);

        if (!simulation_active()) {
            reset_simulation_clock();
            break;
        }
    }
}

void Game::reset_simulation_clock() {
    m_accumulator = 0.0f;
}
