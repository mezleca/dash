#pragma once

#include <imgui-ui/layout/layer-container.hpp>

class Overlay : public ui::LayerContainer {
public:
    explicit Overlay(std::string id, ui::TransitionSpec transition = {0.2F, ui::easing::out_cubic}, bool animate_scale = true);

    virtual void show();
    void hide(bool scale = true);
    void on_key_press(ui::InputCallback callback);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void key_press_event(ui::UiEvent& event) override;

private:
    ui::TransitionSpec m_transition;
    ui::InputCallback m_on_key_press;
    bool m_animate_scale;
};
