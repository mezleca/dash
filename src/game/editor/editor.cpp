#include "editor.hpp"
#include "library.hpp"

#include "core/components/rigid-body.hpp"
#include "core/components/sprite.hpp"
#include "core/components/particles.hpp"
#include "core/viewport/viewport.hpp"
#include "game/level/level.hpp"
#include "game/objects/world/invisible-sprite.hpp"
#include "game/objects/world/image-sprite.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <raymath.h>

static constexpr float MINIMUM_SIZE = 8.0F; // world pixels
static constexpr float SNAP_DISTANCE = 12.0F; // screen pixels
static constexpr Vector2 CORNER_DIRECTIONS[] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

static Rectangle pointer_rectangle(Vector2 start, Vector2 end) {
    return {std::min(start.x, end.x), std::min(start.y, end.y), std::abs(end.x - start.x), std::abs(end.y - start.y)};
}

static Camera2D object_camera(const Entity& object, const Camera2D& camera) {
    const auto* image = object.get_component<ImageSprite>();
    return image != nullptr ? image->render_camera(camera) : camera;
}

static Rectangle object_bounds(const Entity& object, const Viewport& viewport) {
    const Rectangle bounds = object.get_component<RigidBody>()->get_bounding_box();
    const auto* image = object.get_component<ImageSprite>();
    return image != nullptr ? image->render_bounds(bounds, viewport.camera().view(), viewport.size()) : bounds;
}

static bool contains_pointer(const Entity& object, Vector2 screen, const Viewport& viewport) {
    const auto* body = object.get_component<RigidBody>();
    if (body == nullptr) return false;

    const Rectangle bounds = object_bounds(object, viewport);
    const Vector2 world = GetScreenToWorld2D(screen, object_camera(object, viewport.camera().view()));
    const Vector2 center = {bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
    const Vector2 local = Vector2Add(center, Vector2Rotate(Vector2Subtract(world, center), -body->get_rotation() * DEG2RAD));
    return CheckCollisionPointRec(local, bounds);
}

static Rectangle corner_bounds(const std::array<Vector2, 4>& corners) {
    Vector2 minimum = corners.front();
    Vector2 maximum = minimum;
    for (const Vector2 corner : corners) {
        minimum = Vector2Min(minimum, corner);
        maximum = Vector2Max(maximum, corner);
    }
    return {minimum.x, minimum.y, maximum.x - minimum.x, maximum.y - minimum.y};
}

LevelEditor::LevelEditor(Viewport& viewport, DashLevel& level) : m_viewport(viewport), m_level(level) {
    update_selection();
    mark_saved();
    m_current_state = snapshot();
}

LevelEditor::~LevelEditor() {
    cancel_add();
}

void LevelEditor::begin_add(const LibraryEntry& entry) {
    if (m_mode == EditorMode::PickTarget) return;
    finish_interaction();
    cancel_add();

    auto& object = entry.m_setup(m_level);
    auto& sprite = *object.get_component<Sprite>();
    auto& body = *object.get_component<RigidBody>();
    m_preview = &object;
    m_preview_opacity = sprite.opacity;
    m_preview_collidable = body.collision_enabled();
    sprite.opacity = 0.5F;
    if (auto* particles = object.get_component<Particles>()) {
        particles->opacity = 0.5F;
    }
    body.set_collision_enabled(false);
    move_preview(m_viewport.camera().view().target);

    m_previous_mode = m_mode;
    m_mode = EditorMode::Add;
}

void LevelEditor::move_preview(Vector2 world_position) {
    auto& body = *m_preview->get_component<RigidBody>();
    const Rectangle bounds = body.get_bounding_box();
    const Vector2 position = body.get_position();
    body.set_position(
        position.x + world_position.x - bounds.x - bounds.width * 0.5F,
        position.y + world_position.y - bounds.y - bounds.height * 0.5F
    );
}

void LevelEditor::cancel_add() {
    if (m_preview == nullptr) {
        return;
    }

    m_level.remove_object(m_preview);
    m_preview = nullptr;
    m_mode = m_previous_mode;
}

void LevelEditor::mark_changed() {
    m_edit_pending = true;
}

void LevelEditor::commit_changes() {
    if (!m_edit_pending) {
        return;
    }

    // serialize after continuous edits end
    auto state = snapshot();

    if (state != m_current_state) {
        if (m_history.size() == OBJECTS_BUFFER_LIMIT) {
            m_history.erase(m_history.begin());
        }

        m_history.push_back(std::move(m_current_state));
        m_current_state = std::move(state);
    }

    m_edit_pending = false;
    m_changes_pending = true;
}

EditorSnapshot LevelEditor::snapshot() const {
    EditorSnapshot state;
    state.m_solid_background = static_cast<std::uint32_t>(ColorToInt(m_level.solid_background()));
    state.m_objects.reserve(m_level.objects().size());
    for (const auto* object : m_level.objects()) {
        state.m_objects.push_back({object->type(), object->serialize_to_buffer()});
    }

    return state;
}

void LevelEditor::undo() {
    if (m_mode == EditorMode::Add || m_mode == EditorMode::PickTarget) return;
    finish_interaction();
    if (m_history.empty()) {
        return;
    }

    auto state = std::move(m_history.back());
    m_history.pop_back();

    m_level.set_solid_background(GetColor(state.m_solid_background));

    // undo recreates objects, so preserve selected ids
    const auto objects = m_level.objects();
    std::vector<int> selected_ids;

    selected_ids.reserve(m_selection.size());
    for (const auto* object : m_selection)
        selected_ids.push_back(object->id());
    m_selection.clear();
    for (auto* object : objects) {
        m_level.remove_object(object);
    }

    for (const auto& saved : state.m_objects) {
        auto& object = m_level.create_object(saved.m_type);
        object.deserialize_from_buffer(saved.m_state, m_level.file().parent_path());
    }
    for (int id : selected_ids) {
        if (auto* object = m_level.find_object(id)) m_selection.push_back(object);
    }

    m_current_state = std::move(state);
    update_selection();
    m_changes_pending = true;
}

void LevelEditor::copy_selection() {
    if (m_mode == EditorMode::Add || m_mode == EditorMode::PickTarget) return;
    finish_interaction();
    m_copied_objects.clear();
    for (const auto* object : m_selection) {
        m_copied_objects.push_back({object->type(), object->serialize_to_buffer()});
    }
}

void LevelEditor::paste() {
    if (m_mode == EditorMode::Add || m_mode == EditorMode::PickTarget) return;
    finish_interaction();
    if (m_copied_objects.empty()) {
        return;
    }

    m_selection.clear();
    for (const auto& source : m_copied_objects) {
        auto& object = m_level.create_object(source.m_type);
        object.deserialize_from_buffer(source.m_state, m_level.file().parent_path());
        object.assign_new_id();
        m_selection.push_back(&object);
    }

    m_mode = EditorMode::Select;
    update_selection();
    mark_changed();
}

void LevelEditor::delete_selection() {
    if (m_mode == EditorMode::Add || m_mode == EditorMode::PickTarget || m_selection.empty()) return;
    finish_interaction();
    for (auto* object : m_selection) {
        m_level.remove_object(object);
    }
    m_selection.clear();
    mark_changed();
}

void LevelEditor::mark_saved() {
    commit_changes();

    m_saved_state = m_level.serialize_to_buffer();
    m_changes_pending = false;
    m_unsaved_changes = false;
}

bool LevelEditor::has_unsaved_changes() const {
    // defer comparisons until the edit ends
    if (m_edit_pending) {
        return true;
    }

    if (m_changes_pending) {
        m_unsaved_changes = m_level.serialize_to_buffer() != m_saved_state;
        m_changes_pending = false;
    }

    return m_unsaved_changes;
}

Entity* LevelEditor::object_at(Vector2 screen_position) const {
    const auto& objects = m_level.objects();
    Entity* selected = nullptr;

    for (auto* object : objects) {
        if (!contains_pointer(*object, screen_position, m_viewport)) {
            continue;
        }

        if (selected == nullptr || object->z_order() >= selected->z_order()) {
            selected = object;
        }
    }

    return selected;
}

void LevelEditor::begin_pick_target(std::function<void(Entity*)> picked) {
    if (m_mode == EditorMode::PickTarget) return;
    finish_interaction();
    cancel_add();
    m_previous_mode = m_mode;
    m_mode = EditorMode::PickTarget;
    m_target_picked = std::move(picked);
}

void LevelEditor::pick_target(const EditorEvent& event) {
    const Vector2 size = m_viewport.size();

    if (event.m_type == EditorEventType::PointerDown && !CheckCollisionPointRec(event.m_position, {0, 0, size.x, size.y})) {
        return;
    }

    Entity* target = event.m_type == EditorEventType::PointerDown ? object_at(event.m_position) : nullptr;

    if (target != nullptr) {
        const auto* image = target->get_component<ImageSprite>();

        if (image != nullptr && image->space() == ImageSpace::Static) {
            return;
        }
    }

    if (target != nullptr || event.m_type == EditorEventType::Cancel) {
        m_mode = m_previous_mode;
        auto picked = std::move(m_target_picked);

        picked(target);
    }
}

void LevelEditor::place_object(const EditorEvent& event) {
    if (event.m_type == EditorEventType::Cancel) {
        cancel_add();
        return;
    }

    if (event.m_type != EditorEventType::PointerMove && event.m_type != EditorEventType::PointerDown) {
        return;
    }

    const Vector2 size = m_viewport.size();

    if (!CheckCollisionPointRec(event.m_position, {0, 0, size.x, size.y})) {
        return;
    }

    move_preview(GetScreenToWorld2D(event.m_position, m_viewport.camera().view()));

    if (event.m_type == EditorEventType::PointerDown) {
        m_preview->get_component<Sprite>()->opacity = m_preview_opacity;
        if (auto* particles = m_preview->get_component<Particles>()) {
            particles->opacity = 1.0F;
            particles->clear();
        }
        m_preview->get_component<RigidBody>()->set_collision_enabled(m_preview_collidable);
        m_level.add_object(*m_preview);
        m_selection = {m_preview};
        m_preview = nullptr;
        m_mode = EditorMode::Select;
        update_selection();
        mark_changed();
    }
}

void LevelEditor::handle_event(const EditorEvent& event) {
    if (event.m_type == EditorEventType::PointerDown || event.m_type == EditorEventType::PointerUp) {
        if (event.m_button != MOUSE_BUTTON_LEFT && (event.m_button != MOUSE_BUTTON_RIGHT || m_mode != EditorMode::Select)) return;
        if (event.m_type == EditorEventType::PointerUp && event.m_button != m_interaction.m_button) return;
    }

    if (m_mode == EditorMode::PickTarget) {
        pick_target(event);
        return;
    }

    if (m_mode == EditorMode::Add) {
        place_object(event);
        return;
    }

    if (event.m_type == EditorEventType::FinishInteraction) {
        if (m_interaction.m_mode != InteractionMode::None) {
            finish_interaction();
        }

        return;
    }

    if (event.m_type == EditorEventType::Delete) {
        delete_selection();
        return;
    }

    if (m_interaction.m_mode == InteractionMode::None) {
        if (event.m_type == EditorEventType::PointerDown) {
            start_interaction(event);
        } else if (event.m_type == EditorEventType::Scroll && event.m_scroll != 0.0F) {
            auto& camera = m_viewport.camera().view();
            camera.zoom = Clamp(camera.zoom + (event.m_scroll > 0.0F ? 0.125F : -0.125F), 0.1F, 10.0F);
        }
        return;
    }

    if (event.m_type != EditorEventType::PointerMove && event.m_type != EditorEventType::PointerUp) {
        return;
    }

    m_interaction.m_end = event.m_position;
    const auto& camera = m_interaction.m_camera;
    Vector2 delta = Vector2Subtract(event.m_position, m_interaction.m_start);

    switch (m_interaction.m_mode) {
        case InteractionMode::Panning:
            delta =
                Vector2Subtract(GetScreenToWorld2D(event.m_position, camera), GetScreenToWorld2D(m_interaction.m_start, camera));
            m_viewport.camera().set_target(Vector2Subtract(camera.target, delta));
            break;
        case InteractionMode::Moving:
            if (event.m_snap) {
                delta = snap_delta(delta);
            }
            for (const auto& start : m_interaction.m_objects) {
                const Vector2 movement = Vector2Subtract(
                    GetScreenToWorld2D(Vector2Add(m_interaction.m_start, delta), start.m_camera),
                    GetScreenToWorld2D(m_interaction.m_start, start.m_camera)
                );
                start.m_entity->get_component<RigidBody>()->set_position(
                    start.m_position.x + movement.x, start.m_position.y + movement.y
                );
            }
            break;
        case InteractionMode::Resizing:
            delta = Vector2Subtract(
                GetScreenToWorld2D(event.m_position, m_interaction.m_objects.front().m_camera),
                GetScreenToWorld2D(m_interaction.m_start, m_interaction.m_objects.front().m_camera)
            );
            resize_selected(delta);
            break;
        case InteractionMode::Selecting:
        case InteractionMode::None:
            break;
    }

    if (event.m_type == EditorEventType::PointerUp) {
        finish_interaction();
    }
}

void LevelEditor::start_interaction(const EditorEvent& event) {
    commit_changes();
    const Vector2 screen_position = event.m_position;
    m_interaction.m_button = event.m_button;

    if (m_mode == EditorMode::Move || event.m_button == MOUSE_BUTTON_RIGHT) {
        begin_interaction(InteractionMode::Panning, screen_position);
        return;
    }

    const auto* selected = selected_object();

    if (selected != nullptr && selected->get_component<RigidBody>() != nullptr) {
        const auto corners = selection_corners(*selected);
        for (std::size_t index = 0; index < corners.size(); ++index) {
            const Vector2 corner = corners[index];
            if (std::abs(screen_position.x - corner.x) <= JOINT_RADIUS &&
                std::abs(screen_position.y - corner.y) <= JOINT_RADIUS) {
                begin_interaction(InteractionMode::Resizing, screen_position);
                m_interaction.m_resize_direction = CORNER_DIRECTIONS[index];
                return;
            }
        }
    }

    for (auto* entity : selected_objects()) {
        if (contains_pointer(*entity, screen_position, m_viewport)) {
            begin_interaction(InteractionMode::Moving, screen_position);
            return;
        }
    }

    auto* picked = object_at(screen_position);
    m_selection = picked != nullptr ? std::vector<Entity*>{picked} : std::vector<Entity*>{};
    update_selection();
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
        const Rectangle bounds = corner_bounds(selection_corners(*entity));
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

        m_interaction.m_objects.push_back(
            {entity, body->get_position(), body->get_size(), body->get_rotation(), object_camera(*entity, m_interaction.m_camera)}
        );
    }
}

void LevelEditor::resize_selected(Vector2 delta) {
    const auto& start = m_interaction.m_objects.front();
    auto& body = *start.m_entity->get_component<RigidBody>();
    const Vector2 direction = m_interaction.m_resize_direction;
    const Vector2 local_delta = Vector2Multiply(Vector2Rotate(delta, -start.m_rotation * DEG2RAD), direction);
    Vector2 size = Vector2Max(Vector2Add(start.m_size, local_delta), {MINIMUM_SIZE, MINIMUM_SIZE});

    // keep circles and capsules proportional
    if (b2Shape_GetType(body.get_shape()) != b2_polygonShape) {
        const Vector2 scale = Vector2Divide(size, start.m_size);
        const float factor = std::abs(scale.x - 1) >= std::abs(scale.y - 1) ? scale.x : scale.y;
        size = Vector2Scale(start.m_size, std::max(factor, MINIMUM_SIZE / std::min(start.m_size.x, start.m_size.y)));
    }

    // pointer recovery may repeat a resize
    const Vector2 current_size = body.get_size();

    if (size.x != current_size.x || size.y != current_size.y) {
        body.set_size(size, {0.5F, 0.5F});
    }

    // anchor the opposite corner using the accepted size
    const Vector2 offset = Vector2Scale(Vector2Multiply(Vector2Subtract(body.get_size(), start.m_size), direction), 0.5F);
    const Vector2 position = Vector2Add(start.m_position, Vector2Rotate(offset, start.m_rotation * DEG2RAD));
    body.set_position(position.x, position.y);
}

Vector2 LevelEditor::snap_delta(Vector2 delta) const {
    Rectangle moving = m_interaction.m_bounds;
    moving.x += delta.x;
    moving.y += delta.y;
    const float threshold = SNAP_DISTANCE;
    float distance = threshold;
    Vector2 correction = {};
    for (auto* entity : m_level.objects()) {
        if (std::find(selected_objects().begin(), selected_objects().end(), entity) != selected_objects().end()) {
            continue;
        }
        const auto* body = entity->get_component<RigidBody>();
        if (body == nullptr) {
            continue;
        }
        const Rectangle bounds = corner_bounds(selection_corners(*entity));
        const float gap_x = std::max({bounds.x - moving.x - moving.width, moving.x - bounds.x - bounds.width, 0.0F});
        const float gap_y = std::max({bounds.y - moving.y - moving.height, moving.y - bounds.y - bounds.height, 0.0F});
        if (gap_x * gap_x + gap_y * gap_y > threshold * threshold) {
            continue;
        }

        const auto closest = [](float first, float second) { return std::abs(first) <= std::abs(second) ? first : second; };
        const float x = closest(bounds.x - moving.x - moving.width, bounds.x + bounds.width - moving.x);
        const float y = closest(bounds.y - moving.y - moving.height, bounds.y + bounds.height - moving.y);
        if (std::abs(x) <= std::abs(y) && std::abs(x) <= distance) {
            distance = std::abs(x);
            correction = {x, 0};
        } else if (std::abs(y) <= distance) {
            distance = std::abs(y);
            correction = {0, y};
        }
    }
    return Vector2Add(delta, correction);
}

void LevelEditor::finish_interaction() {
    if (const auto rectangle = selection_rectangle()) {
        std::vector<Entity*> objects;
        for (auto* entity : m_level.objects()) {
            const auto* body = entity->get_component<RigidBody>();
            if (body == nullptr) {
                continue;
            }
            const Rectangle bounds = corner_bounds(selection_corners(*entity));
            if (CheckCollisionRecs(bounds, *rectangle)) {
                objects.push_back(entity);
            }
        }
        m_selection = std::move(objects);
        update_selection();
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

    commit_changes();
}

std::optional<Rectangle> LevelEditor::selection_rectangle() const {
    if (m_interaction.m_mode != InteractionMode::Selecting) {
        return std::nullopt;
    }

    return pointer_rectangle(m_interaction.m_start, m_interaction.m_end);
}

std::array<Vector2, 4> LevelEditor::selection_corners(const Entity& object) const {
    const Rectangle bounds = object_bounds(object, m_viewport);
    const auto& body = *object.get_component<RigidBody>();
    const Vector2 center = {bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
    const Camera2D camera = object_camera(object, m_viewport.camera().view());
    std::array<Vector2, 4> corners = {
        {{bounds.x, bounds.y},
         {bounds.x + bounds.width, bounds.y},
         {bounds.x + bounds.width, bounds.y + bounds.height},
         {bounds.x, bounds.y + bounds.height}}
    };
    for (auto& corner : corners) {
        corner = Vector2Add(center, Vector2Rotate(Vector2Subtract(corner, center), body.get_rotation() * DEG2RAD));
        corner = GetWorldToScreen2D(corner, camera);
    }
    return corners;
}

void LevelEditor::set_mode(EditorMode mode) {
    if (m_mode == EditorMode::Add || m_mode == EditorMode::PickTarget || mode == EditorMode::Add ||
        mode == EditorMode::PickTarget || m_mode == mode) {
        return;
    }

    finish_interaction();
    m_mode = mode;
    if (mode == EditorMode::Move) {
        m_selection.clear();
        update_selection();
    }
}

void LevelEditor::reset() {
    if (m_mode == EditorMode::PickTarget) handle_event({EditorEventType::Cancel});
    cancel_add();
    finish_interaction();
    m_selection.clear();
    update_selection();
    m_copied_objects.clear();
    m_history.clear();
    m_current_state = snapshot();
}

bool LevelEditor::clear_selection() {
    if (m_mode == EditorMode::PickTarget) return false;
    finish_interaction();
    const bool had_selection = !selected_objects().empty();
    m_selection.clear();
    update_selection();
    return had_selection;
}

void LevelEditor::update_selection() {
    for (auto* object : m_level.objects()) {
        if (auto* sprite = object->get_component<InvisibleSprite>()) {
            sprite->select(std::find(m_selection.begin(), m_selection.end(), object) != m_selection.end());
        }
    }
}

Entity* LevelEditor::selected_object() const {
    return selected_objects().size() == 1 ? selected_objects().front() : nullptr;
}
