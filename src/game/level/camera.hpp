#pragma once

#include <raylib.h>

class CameraView;
class DashLevel;

enum class CameraFocusMode {
    Object,
    Region,
    Player
};
enum class CameraEasing {
    Linear,
    In,
    Out,
    InOut
};

struct CameraFocusSettings {
    CameraFocusMode m_mode = CameraFocusMode::Object;
    int m_object_id = 0; // zero targets the behaviour's owner
    Vector2 m_position = {};
    Vector2 m_offset = {};
    float m_rotation = 0.0F;
    float m_duration = 0.5F;
    CameraEasing m_easing = CameraEasing::InOut;
};

class LevelCamera {
public:
    explicit LevelCamera(DashLevel& level) : m_level(level) {}
    void focus(const CameraFocusSettings& settings);
    void clear();
    void update(CameraView& camera, Vector2 default_target, float timestep, bool snap = false);

private:
    DashLevel& m_level;
    CameraFocusSettings m_settings;
    Vector2 m_start = {};
    float m_start_rotation = 0.0F;
    float m_elapsed = 0.0F;
    bool m_active = false;
    bool m_pending = false;
};
