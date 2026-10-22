#include "editor.hpp"

#include "core/components/rigid-body.hpp"
#include "core/viewport/viewport.hpp"
#include "game/level/level.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <raymath.h>

static constexpr float MINIMUM_SIZE = 8.0F; // world pixels
static constexpr float SNAP_DISTANCE = 12.0F; // screen pixels
static constexpr Vector2 CORNER_DIRECTIONS[] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

static Rectangle pointer_rectangle(Vector2 start, Vector2 end) {
    return {std::min(start.x, end.x), std::min(start.y, end.y), std::abs(end.x - start.x), std::abs(end.y - start.y)};
}

static Rectangle object_bounds(const RigidBody& body, const Camera2D& camera = {{}, {}, 0, 1}) {
    const Rectangle bounds = body.get_bounding_box();
    const Vector2 center = GetWorldToScreen2D({bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F}, camera);
    const float rotation = (body.get_rotation() + camera.rotation) * DEG2RAD;
    const float cosine = std::abs(std::cos(rotation)) * camera.zoom;
    const float sine = std::abs(std::sin(rotation)) * camera.zoom;
    const Vector2 size = {bounds.width * cosine + bounds.height * sine, bounds.width * sine + bounds.height * cosine};
    return {center.x - size.x * 0.5F, center.y - size.y * 0.5F, size.x, size.y};
}

static Vector2 local_pointer(const RigidBody& body, Vector2 world) {
    const Rectangle bounds = body.get_bounding_box();
    const Vector2 center = {bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
    return Vector2Add(center, Vector2Rotate(Vector2Subtract(world, center), -body.get_rotation() * DEG2RAD));
}

LevelEditor::LevelEditor(Viewport& viewport, DashLevel& level) : m_viewport(viewport), m_level(level) {
    mark_saved();
    m_current_state = snapshot();
}

void LevelEditor::mark_changed() {
    m_level.update_end_position();
    auto state = snapshot();

    if (state != m_current_state) {
        if (m_history.size() == OBJECTS_BUFFER_LIMIT) m_history.erase(m_history.begin());
        m_history.push_back(std::move(m_current_state));
        m_current_state = std::move(state);
    }

    m_changes_pending = true;
}

EditorSnapshot LevelEditor::snapshot() const {
    EditorSnapshot state;
    state.m_objects = m_level.objects();
    state.m_states.reserve(state.m_objects.size());

    for (const auto* object : state.m_objects) {
        state.m_states.push_back(object->serialize_to_buffer());
    }

    return state;
}

void LevelEditor::undo() {
    pointer_up();
    if (m_history.empty()) {
        return;
    }

    auto state = std::move(m_history.back());
    m_history.pop_back();
    const auto objects = m_level.objects();

    for (auto* object : objects) {
        if (std::find(state.m_objects.begin(), state.m_objects.end(), object) == state.m_objects.end()) {
            std::erase(m_selection, object);
            std::erase(m_copied_objects, object);
            m_level.remove_object(object);
        }
    }

    for (std::size_t index = 0; index < state.m_objects.size(); ++index) {
        state.m_objects[index]->deserialize_from_buffer(state.m_states[index], m_level.file().parent_path());
    }

    m_current_state = std::move(state);
    m_level.update_end_position();
    m_changes_pending = true;
}

void LevelEditor::copy_selection() {
    pointer_up();
    m_copied_objects = m_selection;
}

void LevelEditor::paste() {
    pointer_up();
    if (m_copied_objects.empty()) {
        return;
    }

    m_selection.clear();
    for (const auto* source : m_copied_objects) {
        auto& object = m_level.create_object(source->type());
        object.deserialize_from_buffer(source->serialize_to_buffer(), m_level.file().parent_path());
        m_selection.push_back(&object);
    }

    m_mode = EditorMode::Select;
    mark_changed();
}

void LevelEditor::mark_saved() {
    m_saved_state = m_level.serialize_to_buffer();
    m_changes_pending = false;
    m_unsaved_changes = false;
}

bool LevelEditor::has_unsaved_changes() const {
    if (m_changes_pending) {
        m_unsaved_changes = m_level.serialize_to_buffer() != m_saved_state;
        m_changes_pending = false;
    }

    return m_unsaved_changes;
}

Entity* LevelEditor::object_at(Vector2 world_position) const {
    const auto& objects = m_level.objects();
    Entity* selected = nullptr;

    for (auto* object : objects) {
        const auto* body = object->get_component<RigidBody>();
        if (body == nullptr || !CheckCollisionPointRec(local_pointer(*body, world_position), body->get_bounding_box())) {
            continue;
        }

        if (selected == nullptr || object->z_order() >= selected->z_order()) {
            selected = object;
        }
    }

    return selected;
}

void LevelEditor::update(bool pointer_over_ui, bool debugger_open) {
    if (debugger_open || !IsWindowFocused()) {
        pointer_up();
        return;
    }

    const Vector2 pointer = m_viewport.mouse_position();
    const bool snap = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        pointer_move(pointer, snap);
        pointer_up();
    }

    if (manipulating_object() || m_interaction.m_mode == InteractionMode::Selecting) {
        pointer_move(pointer, snap);
        return;
    }

    if (pointer_over_ui || !m_viewport.contains_mouse()) {
        pointer_up();
        return;
    }

    auto& camera = m_viewport.camera();
    const float wheel = GetMouseWheelMoveV().y;

    if (wheel != 0.0F) {
        float target_zoom = camera.view().zoom;
        target_zoom += (wheel > 0) ? 0.125f : -0.125f;
        camera.view().zoom = Clamp(target_zoom, 0.1F, 10.0F);
    }

    pointer_move(pointer, snap);
}

void LevelEditor::pointer_down(Vector2 screen_position) {
    pointer_up();
    if (m_mode == EditorMode::Move) {
        begin_interaction(InteractionMode::Panning, screen_position);
        return;
    }

    const Vector2 world = GetScreenToWorld2D(screen_position, m_viewport.camera().view());
    const auto* selected = selected_object();

    if (const auto* body = selected != nullptr ? selected->get_component<RigidBody>() : nullptr) {
        const Rectangle bounds = body->get_bounding_box();
        const Vector2 center = {bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
        const Vector2 local = local_pointer(*body, world);

        const float radius = JOINT_RADIUS / m_viewport.camera().view().zoom;
        for (const Vector2 direction : CORNER_DIRECTIONS) {
            const Vector2 corner = {center.x + bounds.width * direction.x * 0.5F, center.y + bounds.height * direction.y * 0.5F};
            if (std::abs(local.x - corner.x) <= radius && std::abs(local.y - corner.y) <= radius) {
                begin_interaction(InteractionMode::Resizing, screen_position);
                m_interaction.m_resize_direction = direction;
                return;
            }
        }
    }

    for (auto* entity : selected_objects()) {
        const auto* body = entity->get_component<RigidBody>();
        if (body != nullptr && CheckCollisionPointRec(local_pointer(*body, world), body->get_bounding_box())) {
            begin_interaction(InteractionMode::Moving, screen_position);
            return;
        }
    }

    auto* picked = object_at(world);
    m_selection = picked != nullptr ? std::vector<Entity*>{picked} : std::vector<Entity*>{};
    begin_interaction(picked != nullptr ? InteractionMode::Moving : InteractionMode::Selecting, screen_position);
}

void LevelEditor::begin_interaction(InteractionMode mode, Vector2 screen_position) {
    m_interaction.m_mode = mode;
    m_interaction.m_start = m_interaction.m_end = screen_position;
    m_interaction.m_camera = m_viewport.camera().view();
    m_interaction.m_objects.clear();
    if (!manipulating_object()) {
        return;
    }

    for (auto* entity : selected_objects()) {
        const auto* body = entity->get_component<RigidBody>();
        const Rectangle bounds = object_bounds(*body);
        if (m_interaction.m_objects.empty()) {
            m_interaction.m_bounds = bounds;
        } else {
            const Rectangle previous = m_interaction.m_bounds;
            m_interaction.m_bounds = pointer_rectangle(
                {std::min(previous.x, bounds.x), std::min(previous.y, bounds.y)},
                {std::max(previous.x + previous.width, bounds.x + bounds.width),
                 std::max(previous.y + previous.height, bounds.y + bounds.height)}
            );
        }
        m_interaction.m_objects.push_back({entity, body->get_position(), body->get_size(), body->get_rotation()});
    }
}

void LevelEditor::pointer_move(Vector2 screen_position, bool snap) {
    m_interaction.m_end = screen_position;
    if (m_interaction.m_mode == InteractionMode::None || m_interaction.m_mode == InteractionMode::Selecting) {
        return;
    }

    const auto& camera = m_interaction.m_camera;
    Vector2 delta =
        Vector2Subtract(GetScreenToWorld2D(screen_position, camera), GetScreenToWorld2D(m_interaction.m_start, camera));
    if (m_interaction.m_mode == InteractionMode::Panning) {
        m_viewport.camera().set_target(Vector2Subtract(camera.target, delta));
        return;
    }

    if (m_interaction.m_mode == InteractionMode::Resizing) {
        resize_selected(delta);
        return;
    }

    if (snap) {
        delta = snap_delta(delta);
    }
    for (const auto& start : m_interaction.m_objects) {
        start.m_entity->get_component<RigidBody>()->set_position(start.m_position.x + delta.x, start.m_position.y + delta.y);
    }
}

void LevelEditor::resize_selected(Vector2 delta) {
    const auto& start = m_interaction.m_objects.front();
    auto& body = *start.m_entity->get_component<RigidBody>();
    const Vector2 direction = m_interaction.m_resize_direction;
    const Vector2 local_delta = Vector2Multiply(Vector2Rotate(delta, -start.m_rotation * DEG2RAD), direction);
    Vector2 size = Vector2Max(Vector2Add(start.m_size, local_delta), {MINIMUM_SIZE, MINIMUM_SIZE});

    // circles and capsules scale uniformly, even after the pointer crosses the fixed corner
    if (b2Shape_GetType(body.get_shape()) != b2_polygonShape) {
        const Vector2 scale = Vector2Divide(size, start.m_size);
        const float factor = std::abs(scale.x - 1) >= std::abs(scale.y - 1) ? scale.x : scale.y;
        size = Vector2Scale(start.m_size, std::max(factor, MINIMUM_SIZE / std::min(start.m_size.x, start.m_size.y)));
    }

    body.set_size(size, {0.5F, 0.5F});

    // use the accepted size to keep the opposite corner fixed if physics rejects a resize
    const Vector2 offset = Vector2Scale(Vector2Multiply(Vector2Subtract(body.get_size(), start.m_size), direction), 0.5F);
    const Vector2 position = Vector2Add(start.m_position, Vector2Rotate(offset, start.m_rotation * DEG2RAD));
    body.set_position(position.x, position.y);
}

Vector2 LevelEditor::snap_delta(Vector2 delta) const {
    Rectangle moving = m_interaction.m_bounds;
    moving.x += delta.x;
    moving.y += delta.y;
    Rectangle nearest = {};
    float distance = std::numeric_limits<float>::infinity();
    for (auto* entity : m_level.objects()) {
        if (std::find(selected_objects().begin(), selected_objects().end(), entity) != selected_objects().end()) {
            continue;
        }
        const auto* body = entity->get_component<RigidBody>();
        if (body == nullptr) {
            continue;
        }
        const Rectangle bounds = object_bounds(*body);
        const float gap_x = std::max({bounds.x - moving.x - moving.width, moving.x - bounds.x - bounds.width, 0.0F});
        const float gap_y = std::max({bounds.y - moving.y - moving.height, moving.y - bounds.y - bounds.height, 0.0F});
        const float candidate = gap_x * gap_x + gap_y * gap_y;
        if (candidate < distance) {
            distance = candidate;
            nearest = bounds;
        }
    }

    const float threshold = SNAP_DISTANCE / m_interaction.m_camera.zoom;
    if (distance > threshold * threshold) {
        return delta;
    }

    const auto closest = [](float first, float second) { return std::abs(first) <= std::abs(second) ? first : second; };
    const float x = closest(nearest.x - moving.x - moving.width, nearest.x + nearest.width - moving.x);
    const float y = closest(nearest.y - moving.y - moving.height, nearest.y + nearest.height - moving.y);
    if (std::abs(x) <= std::abs(y) && std::abs(x) <= threshold) {
        delta.x += x;
    } else if (std::abs(y) <= threshold) {
        delta.y += y;
    }
    return delta;
}

void LevelEditor::pointer_up() {
    if (const auto rectangle = selection_rectangle()) {
        std::vector<Entity*> objects;
        for (auto* entity : m_level.objects()) {
            const auto* body = entity->get_component<RigidBody>();
            if (body == nullptr) {
                continue;
            }
            const Rectangle bounds = object_bounds(*body, m_interaction.m_camera);
            if (CheckCollisionRecs(bounds, *rectangle)) {
                objects.push_back(entity);
            }
        }
        m_selection = std::move(objects);
    }

    for (const auto& start : m_interaction.m_objects) {
        const auto* body = start.m_entity->get_component<RigidBody>();
        if (!Vector2Equals(body->get_position(), start.m_position) || !Vector2Equals(body->get_size(), start.m_size)) {
            mark_changed();
            break;
        }
    }
    m_interaction.m_mode = InteractionMode::None;
    m_interaction.m_objects.clear();
}

std::optional<Rectangle> LevelEditor::selection_rectangle() const {
    if (m_interaction.m_mode != InteractionMode::Selecting) {
        return std::nullopt;
    }

    return pointer_rectangle(m_interaction.m_start, m_interaction.m_end);
}

void LevelEditor::set_mode(EditorMode mode) {
    if (m_mode == mode) {
        return;
    }

    pointer_up();
    m_mode = mode;
    if (mode == EditorMode::Move) {
        m_selection.clear();
    }
}

void LevelEditor::reset() {
    pointer_up();
    m_selection.clear();
    m_copied_objects.clear();
    m_history.clear();
    m_current_state = snapshot();
}

bool LevelEditor::clear_selection() {
    pointer_up();
    const bool had_selection = !selected_objects().empty();
    m_selection.clear();
    return had_selection;
}

Entity* LevelEditor::selected_object() const {
    return selected_objects().size() == 1 ? selected_objects().front() : nullptr;
}
