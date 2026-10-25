#pragma once

#include <imgui-ui/layout/container.hpp>
#include <functional>
#include <string>
#include "game/ui/widgets/dash-number-input.hpp"
#include "game/level/camera.hpp"

class Entity;
class Behaviour;
class CameraFocus;

class BehaviourEditor : public ui::Container {
public:
    BehaviourEditor(
        Entity& object, Behaviour& behaviour, std::function<void()> changed, std::function<void()> rebuild,
        std::function<void(CameraFocus&)> select_target
    );

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    DashNumberInput<float>& add_number(float& value, const char* label, std::function<void()> changed);
    void build_camera_focus(
        Entity& object, CameraFocus& focus, std::function<void()> changed, std::function<void()> rebuild,
        std::function<void(CameraFocus&)> select_target
    );

    float m_period = 0.0F;
    float m_minimum_opacity = 0.0F;
    CameraFocusSettings m_focus_settings;
    std::string m_focus_mode;
    std::string m_focus_easing;
    std::string m_player_type;
};
