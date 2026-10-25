#include "camera-focus.hpp"
#include "game/level/level.hpp"

#include <stdexcept>

CameraFocus::CameraFocus(Entity& owner, DashLevel& level) : m_owner(owner), m_level(level) {}

void CameraFocus::set_settings(const CameraFocusSettings& settings) {
    if (settings.m_mode < CameraFocusMode::Object || settings.m_mode > CameraFocusMode::Player ||
        settings.m_easing < CameraEasing::Linear || settings.m_easing > CameraEasing::InOut || settings.m_object_id < 0 ||
        settings.m_duration < 0.0F) {
        throw std::invalid_argument("invalid camera focus settings");
    }

    m_settings = settings;
}

void CameraFocus::update(float) {
    auto settings = m_settings;
    if (settings.m_mode == CameraFocusMode::Object && settings.m_object_id == 0) {
        settings.m_object_id = m_owner.id();
    }

    m_level.camera_control().focus(settings);
    finished();
}

void CameraFocus::serialize(Serializer& serializer) const {
    serializer.field("mode", static_cast<int>(m_settings.m_mode));
    serializer.field("object_id", m_settings.m_object_id);
    serializer.field("position", m_settings.m_position);
    serializer.field("offset", m_settings.m_offset);
    serializer.field("rotation", m_settings.m_rotation);
    serializer.field("duration", m_settings.m_duration);
    serializer.field("easing", static_cast<int>(m_settings.m_easing));
}

void CameraFocus::deserialize(Deserializer& deserializer) {
    CameraFocusSettings settings;

    int mode = static_cast<int>(settings.m_mode);
    int easing = static_cast<int>(settings.m_easing);

    deserializer.field("mode", mode);
    deserializer.field("object_id", settings.m_object_id);
    deserializer.field("position", settings.m_position);
    deserializer.field("offset", settings.m_offset);
    deserializer.field("rotation", settings.m_rotation);
    deserializer.field("duration", settings.m_duration);
    deserializer.field("easing", easing);

    settings.m_mode = static_cast<CameraFocusMode>(mode);
    settings.m_easing = static_cast<CameraEasing>(easing);

    set_settings(settings);
}
