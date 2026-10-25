#pragma once

#include <imgui-ui/layout/layer-container.hpp>
#include "game/ui/theme.hpp"
#include <functional>
#include <string>

class Overlay : public ui::LayerContainer {
public:
    explicit Overlay(
        std::string id, ui::TransitionSpec transition = dash_ui::panel_transition, bool animate_scale = true,
        bool with_header = false, std::string header_title = {}
    );

    virtual void show();
    void hide(bool scale = true);
    void on_key_press(ui::InputCallback callback);
    void on_close(std::function<void()> callback);

protected:
    ui::Container& content() {
        return *m_content;
    }
    void apply_theme_defaults(const ui::Theme& theme) override;
    void key_press_event(ui::UiEvent& event) override;

private:
    ui::Container* m_content = nullptr;
    ui::TransitionSpec m_transition;
    ui::InputCallback m_on_key_press;
    std::function<void()> m_on_close;
    bool m_animate_scale;
};
