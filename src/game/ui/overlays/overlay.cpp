#include "overlay.hpp"
#include "game/ui/widgets/modal-header.hpp"
#include "game/ui/theme.hpp"

#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

Overlay::Overlay(std::string id, TransitionSpec transition, bool animate_scale, bool with_header, std::string header_title)
    : LayerContainer(std::move(id)), m_transition(transition), m_animate_scale(animate_scale) {
    set_size({fit(), fit()});
    set_anchor(Anchor::Center);

    // block underlying pointer input and route keys to this subtree.
    set_input_mode(InputMode::Blocker);
    set_input_enabled(false);
    set_visible(false);

    if (with_header) {
        add<DashModalHeader>(std::move(header_title), [this] {
            if (m_on_close) m_on_close();
        });
    }
    m_content = &add<Container>("overlay-content");
    m_content->set_size({grow(), grow()});

    configure_all_styles([](Style& style) { style.scale({1.0F, 1.0F}); });
}

void Overlay::on_key_press(InputCallback callback) {
    m_on_key_press = std::move(callback);
}

void Overlay::on_close(std::function<void()> callback) {
    m_on_close = std::move(callback);
}

void Overlay::key_press_event(UiEvent& event) {
    if (m_on_key_press) m_on_key_press(event);
}

void Overlay::show() {
    cancel_animations();
    set_input_enabled(true);
    set_visible(true);
    fade_in(m_transition);

    if (!m_animate_scale) {
        return;
    }

    animate()
        .to(StyleAnimationProperty::Scale, ImVec2{0.0F, 0.0F})
        .then()
        .to(StyleAnimationProperty::Scale, ImVec2{1.0F, 1.0F}, m_transition);
}

void Overlay::hide(bool scale) {
    cancel_animations();
    set_input_enabled(false);
    fade_out(m_transition);

    auto sequence = animate();
    if (scale && m_animate_scale) {
        sequence.to(StyleAnimationProperty::Scale, ImVec2{0.0F, 0.0F}, m_transition);
    }

    sequence.delay(m_transition.duration).end([this] { set_visible(false); });
}

void Overlay::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);
    configure_all_styles([&theme](Style& style) { dash_ui::panel_style(style, theme); });
    set_font(surface().get_primary_font(), 28);
}
