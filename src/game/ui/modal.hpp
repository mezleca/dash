#pragma once

#include <imgui-ui/layout/layer-container.hpp>
#include <functional>

namespace ui {
    class TextWidget;
}
class DashModalHeader;

enum class ModalState {
    Closed,
    Open,
    Minimized
};

class Modal : public ui::LayerContainer {
public:
    explicit Modal(std::string id);
    void open();
    virtual void close();
    void minimize();
    void restore();
    ModalState state() const {
        return m_state;
    }
    void on_state_change(std::function<void(ModalState)> callback);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void event(ui::UiEvent& event) override;
    void set_title(std::string title);
    ui::Container* m_body = nullptr;
    ui::Container* m_actions = nullptr;

private:
    void set_state(ModalState state);

    ui::Container* m_panel = nullptr;
    DashModalHeader* m_header = nullptr;
    ModalState m_state = ModalState::Closed;
    std::function<void(ModalState)> m_state_changed;
};
