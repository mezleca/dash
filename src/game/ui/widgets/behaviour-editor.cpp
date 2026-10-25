#include "behaviour-editor.hpp"

#include "game/behaviours/pulse.hpp"
#include "game/behaviours/camera-focus.hpp"
#include "game/behaviours/player-type.hpp"
#include "game/level/level.hpp"
#include "game/objects/entities/entity.hpp"
#include "game/ui/controls.hpp"
#include "game/ui/widgets/dash-number-input.hpp"
#include "dash-vector-input.hpp"

#include <utility>
#include <vector>

using namespace ui;

BehaviourEditor::BehaviourEditor(
    Entity& object, Behaviour& behaviour, std::function<void()> changed, std::function<void()> rebuild,
    std::function<void(CameraFocus&)> select_target
)
    : Container({}, "BehaviourEditor") {
    set_size({grow(), fit()});
    set_spacing(4.0F);

    if (auto* pulse = dynamic_cast<Pulse*>(&behaviour)) {
        m_period = pulse->period();
        add_number(m_period, "period (seconds)", [this, pulse, changed] {
            pulse->set_period(m_period);
            changed();
        }).set_minimum(0.01F);

        m_minimum_opacity = pulse->minimum_opacity();
        auto& opacity = add_number(m_minimum_opacity, "minimum opacity", [this, pulse, changed] {
            pulse->set_minimum_opacity(m_minimum_opacity);
            changed();
        });
        opacity.set_minimum(0.0F);
        opacity.set_maximum(1.0F);
    } else if (auto* focus = dynamic_cast<CameraFocus*>(&behaviour)) {
        build_camera_focus(object, *focus, changed, rebuild, std::move(select_target));
    } else if (auto* type = dynamic_cast<SetPlayerType*>(&behaviour)) {
        m_player_type = std::to_string(static_cast<int>(type->player_type()));
        auto& field = add<DashDropdown>(m_player_type, std::vector<DropdownOption>{{"box", "0"}, {"bird", "1"}}, "player type");
        field.set_size({grow(), fit()});
        field.on_change([this, type, changed] {
            type->set_player_type(static_cast<PlayerType>(std::stoi(m_player_type)));
            changed();
        });
    }
}

DashNumberInput<float>& BehaviourEditor::add_number(float& value, const char* label, std::function<void()> changed) {
    auto& field = add<DashNumberInput<float>>(value);
    field.set_size({grow(), fit()});
    field.set_label(label, LabelPlacement::Above);
    field.clear_range();
    field.on_change(std::move(changed));
    return field;
}

void BehaviourEditor::build_camera_focus(
    Entity& object, CameraFocus& focus, std::function<void()> changed, std::function<void()> rebuild,
    std::function<void(CameraFocus&)> select_target
) {
    m_focus_settings = focus.settings();
    m_focus_mode = std::to_string(static_cast<int>(m_focus_settings.m_mode));
    auto& mode = add<DashDropdown>(
        m_focus_mode, std::vector<DropdownOption>{{"object", "0"}, {"world region", "1"}, {"player", "2"}}, "target"
    );
    mode.set_size({grow(), fit()});
    mode.on_change([this, &focus, rebuild] {
        m_focus_settings.m_mode = static_cast<CameraFocusMode>(std::stoi(m_focus_mode));
        focus.set_settings(m_focus_settings);
        rebuild();
    });

    const auto apply = [this, &focus, changed] {
        focus.set_settings(m_focus_settings);
        changed();
    };
    if (m_focus_settings.m_mode == CameraFocusMode::Object) {
        const auto* selected = object.level()->find_object(m_focus_settings.m_object_id);
        auto& target = add<DashTargetButton>(selected != nullptr ? selected->type_to_str() : "select");
        target.set_size({grow(), fit()});
        target.on_click([&focus, select_target] { select_target(focus); });
    } else if (m_focus_settings.m_mode == CameraFocusMode::Region) {
        add<DashVectorInput>(m_focus_settings.m_position, "position").on_change(apply);
    }

    add<DashVectorInput>(m_focus_settings.m_offset, "offset").on_change(apply);
    add_number(m_focus_settings.m_rotation, "rotation offset", apply);
    add_number(m_focus_settings.m_duration, "transition (seconds)", apply).set_minimum(0.0F);
    m_focus_easing = std::to_string(static_cast<int>(m_focus_settings.m_easing));
    auto& easing = add<DashDropdown>(
        m_focus_easing, std::vector<DropdownOption>{{"linear", "0"}, {"ease in", "1"}, {"ease out", "2"}, {"ease in/out", "3"}},
        "easing"
    );
    easing.set_size({grow(), fit()});
    easing.on_change([this, &focus, changed] {
        m_focus_settings.m_easing = static_cast<CameraEasing>(std::stoi(m_focus_easing));
        focus.set_settings(m_focus_settings);
        changed();
    });
}

void BehaviourEditor::apply_theme_defaults(const Theme& theme) {
    Container::apply_theme_defaults(theme);
    configure_all_styles([](Style& style) {
        style.padding({});
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
    });
}
