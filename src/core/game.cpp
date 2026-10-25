#include "game.hpp"
#include "components/rigid-body.hpp"

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
        // detach first because destructors can queue more removals.
        auto removed = std::move(*entry);
        m_objects.erase(entry);
        std::erase(m_render_objects, removed.get());
        removed.reset();
        entry = std::find_if(m_objects.begin(), m_objects.end(), [](const auto& object) { return object->m_removed; });
    }
}

void Game::update() {
    const std::size_t count = m_objects.size();

    for (std::size_t index = 0; index < count; ++index) {
        GameObject* object = m_objects[index].get();
        if (!object->m_removed) {
            object->update(GetFrameTime());
        }
    }
}

void Game::render() {
    const auto compare = [](const GameObject* a, const GameObject* b) { return a->z_order() < b->z_order(); };

    if (!std::is_sorted(m_render_objects.begin(), m_render_objects.end(), compare)) {
        std::stable_sort(m_render_objects.begin(), m_render_objects.end(), compare);
    }

    for (const auto& viewport : m_viewports) {
        viewport->begin();
        render_viewport(*viewport);
        viewport->end();
    }
}

void Game::render_viewport(Viewport& viewport) {
    auto& camera = viewport.camera();

    BeginMode2D(camera.view());
    for (const GameObject* object : m_render_objects) {
        if (object->m_removed || !object->visible || (object->render_mask() & viewport.mask()) == 0) {
            continue;
        }

        const auto* body = object->get_component<RigidBody>();
        const Rectangle bounds = body != nullptr ? body->get_bounding_box() : Rectangle{};
        object->render(bounds, camera.view(), viewport.size());
    }
    EndMode2D();
}

Viewport& Game::create_viewport(ViewportId id) {
    return *m_viewports.emplace_back(std::make_unique<Viewport>(id));
}

Viewport* Game::get_viewport(size_t index) const {
    return index < m_viewports.size() ? m_viewports[index].get() : nullptr;
}

CameraView* Game::camera_for(const GameObject& object) const {
    for (const auto& viewport : m_viewports) {
        if ((object.render_mask() & viewport->mask()) != 0) {
            return &viewport->camera();
        }
    }

    return nullptr;
}

void Game::run() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_window.width, m_window.height, m_window.title.c_str());
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(0);

    for (const auto& viewport : m_viewports) {
        if (viewport->size().x <= 0.0f) {
            viewport->set_size({static_cast<float>(m_window.width), static_cast<float>(m_window.height)});
        }
    }

    on_initialize();

    while (!m_done) {
        update_simulation_timestep();

        if (IsWindowResized()) {
            m_window.width = GetScreenWidth();
            m_window.height = GetScreenHeight();
        }

        const float frametime = GetFrameTime();

        on_update(frametime);
        update();

        render();
        on_render();

        remove_pending_objects();
    }

    on_shutdown();

    clear_objects();
    remove_pending_objects();

    CloseAudioDevice();
    CloseWindow();
}

void Game::exit() {
    m_done = true;
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
