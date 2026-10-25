#include "modal-header.hpp"

#include "game/ui/controls.hpp"

#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

DashModalHeader::DashModalHeader(std::string title, std::function<void()> on_close) : Container("modal-header") {
    set_size({grow(), px(30.0F)});
    set_input_mode(InputMode::None);
    set_content_alignment(Anchor::CenterRight);
    set_title(std::move(title));

    if (on_close) add<DashCloseButton>().on_click(std::move(on_close));
}

void DashModalHeader::set_title(std::string title) {
    if (title.empty()) {
        if (m_title_slot != nullptr) m_title_slot->set_visible(false);
        return;
    }

    if (m_title == nullptr) {
        m_title_slot = &add<Container>("modal-header-title");
        m_title_slot->set_size({percent(70.0F), grow()});
        m_title_slot->set_anchor(Anchor::Center);
        m_title_slot->set_content_alignment(Anchor::Center);
        m_title_slot->set_input_mode(InputMode::None);
        m_title = &m_title_slot->add<TextWidget>(std::move(title));
    } else {
        m_title->set_text(std::move(title));
    }

    m_title_slot->set_visible(true);
}
