#include "modal.hpp"
#include "game/ui/widgets/modal-header.hpp"
#include "theme.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

Modal::Modal(std::string id) : LayerContainer(std::move(id)) {
    set_size({grow(), grow()});
    set_visible(false);
    set_input_enabled(false);
    set_input_mode(InputMode::Blocker);
    configure_all_styles([](Style& style) {
        style.padding({});
        style.border(BorderEdge{0.0F, BorderType::None});
    });
    m_panel = &add<Container>("modal-panel");
    m_panel->set_spacing(dash_ui::section_spacing);
    m_header = &m_panel->add<DashModalHeader>("", [this] { close(); });
    m_body = &m_panel->add<Container>("modal-body");
    m_body->set_size({grow(), grow()});
    m_body->set_scrollable(true);
    m_actions = &m_panel->add<Container>("modal-actions", StackDirection::Horizontal);
    m_actions->set_size({grow(), fit()});
    m_actions->set_spacing(8.0F);
}

void Modal::apply_theme_defaults(const Theme& theme) {
    LayerContainer::apply_theme_defaults(theme);
    m_panel->configure_all_styles([&theme](Style& style) { dash_ui::panel_style(style, theme); });
}

void Modal::set_title(std::string title) {
    m_header->set_title(std::move(title));
}

void Modal::on_state_change(std::function<void(ModalState)> callback) {
    m_state_changed = std::move(callback);
}

void Modal::open() {
    set_state(ModalState::Open);
}
void Modal::close() {
    set_state(ModalState::Closed);
}
void Modal::minimize() {
    set_state(ModalState::Minimized);
}
void Modal::restore() {
    set_state(ModalState::Open);
}

void Modal::set_state(ModalState state) {
    m_state = state;
    const bool minimized = state == ModalState::Minimized;

    set_visible(state != ModalState::Closed);
    set_input_enabled(state != ModalState::Closed);
    set_anchor(minimized ? Anchor::TopRight : Anchor::TopLeft);
    set_size(minimized ? LayoutSize{px(280), fit()} : LayoutSize{grow(), grow()});
    configure_all_styles([minimized](Style& style) { style.background_color(rgba(0, 0, 0, minimized ? 0 : 160)); });

    m_panel->set_input_mode(InputMode::Target);
    m_panel->set_layout({.size = minimized ? LayoutSize{grow(), fit()} : LayoutSize{percent(70), percent(85)}});

    if (!minimized) {
        m_panel->set_anchor(Anchor::Center);
    }

    m_body->set_visible(!minimized);
    m_actions->set_visible(!minimized);

    if (m_state_changed) {
        m_state_changed(state);
    }

    if (state != ModalState::Closed) {
        surface().input_router().set_focus(this);
    }
}

void Modal::event(UiEvent& input) {
    if ((input.type == EventType::KeyDown && input.key == Key::Escape) ||
        (input.type == EventType::Click && input.target == this && m_state == ModalState::Open)) {
        close();
        input.stop_propagation();
        input.block_native_input();
    }
}
