#include "camera.hpp"

#include "level.hpp"
#include "core/camera.hpp"
#include "core/components/rigid-body.hpp"
#include "core/math.hpp"
#include "game/dash.hpp"
#include "game/objects/world/image-sprite.hpp"

#include <algorithm>
#include <cmath>
#include <raymath.h>

void LevelCamera::focus(const CameraFocusSettings& settings) {
    m_settings = settings;
    m_pending = true;
}

void LevelCamera::clear() {
    m_active = false;
    m_pending = false;
    m_elapsed = 0.0F;
}

void LevelCamera::update(CameraView& camera, Vector2 default_target, float timestep, bool snap) {
    if (snap) {
        clear();
        camera.set_target(default_target);
        camera.view().rotation = DEFAULT_CAMERA_ROTATION;
        return;
    }

    if (m_pending) {
        m_start = camera.view().target;
        m_start_rotation = camera.view().rotation;
        m_elapsed = 0.0F;
        m_pending = false;
        m_active = true;
    }

    Vector2 target = default_target;
    if (m_active && m_settings.m_mode == CameraFocusMode::Object) {
        const auto* object = m_level.find_object(m_settings.m_object_id);
        const auto* body = object != nullptr ? object->get_component<RigidBody>() : nullptr;
        const auto* image = object != nullptr ? object->get_component<ImageSprite>() : nullptr;

        if (body == nullptr || (image != nullptr && image->space() == ImageSpace::Static)) {
            clear();
        } else {
            const Rectangle bounds = body->get_bounding_box();
            target = {bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
        }
    } else if (m_active && m_settings.m_mode == CameraFocusMode::Region) {
        target = m_settings.m_position;
    }

    if (!m_active) {
        camera.set_target(default_target, {1.0F, 0.09F});
        camera.view().rotation = DEFAULT_CAMERA_ROTATION;
        return;
    }

    target = Vector2Add(target, m_settings.m_offset);
    const float rotation = DEFAULT_CAMERA_ROTATION + m_settings.m_rotation;
    m_elapsed += timestep;
    const float progress = m_settings.m_duration > 0.0F ? std::min(m_elapsed / m_settings.m_duration, 1.0F) : 1.0F;
    float eased = progress;

    switch (m_settings.m_easing) {
        case CameraEasing::Linear:
            break;
        case CameraEasing::In:
            eased = d_easing::in(progress);
            break;
        case CameraEasing::Out:
            eased = d_easing::out(progress);
            break;
        case CameraEasing::InOut:
            eased = d_easing::in_out(progress);
            break;
    }

    camera.set_target(Vector2Lerp(m_start, target, eased));
    camera.view().rotation = m_start_rotation + std::remainder(rotation - m_start_rotation, 360.0F) * eased;
}
