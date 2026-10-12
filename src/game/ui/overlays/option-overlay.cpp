#include "option-overlay.hpp"

#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

MenuOptionOverlay::MenuOptionOverlay(std::string id, TransitionSpec transition, bool animate_scale)
    : LayerContainer(std::move(id)), m_transition(transition), m_animate_scale(animate_scale) {
    set_size({fit(), fit()});
    set_anchor(Anchor::Center);

    // block pointer input behind the panel and route keyboard input to its subtree.
    set_input_mode(InputMode::Blocker);
    set_input_enabled(false);
    set_visible(false);

    configure_all_styles([](Style& style) {
        style.scale({1.0F, 1.0F});
        style.padding({16.0F, 16.0F});
        style.background_color(rgb(16, 16, 16));
        style.border(BorderEdge{2.0F, BorderType::Solid});
        style.border_color(rgba(160, 160, 160, 60));
    });
}

void MenuOptionOverlay::on_key_press(InputCallback callback) {
    m_on_key_press = std::move(callback);
}

void MenuOptionOverlay::key_press_event(UiEvent& event) {
    if (m_on_key_press) m_on_key_press(event);
}

void MenuOptionOverlay::show() {
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

void MenuOptionOverlay::hide(bool scale) {
    cancel_animations();
    set_input_enabled(false);
    fade_out(m_transition);

    auto sequence = animate();
    if (scale && m_animate_scale) {
        sequence.to(StyleAnimationProperty::Scale, ImVec2{0.0F, 0.0F}, m_transition);
    }

    sequence.delay(m_transition.duration).end([this] { set_visible(false); });
}

void MenuOptionOverlay::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 28);
}
