#pragma once

#include <imgui-ui/layout/layer-container.hpp>

class MenuOptionLayer : public ui::LayerContainer {
public:
    explicit MenuOptionLayer(
        std::string id, ui::TransitionSpec transition = {0.2F, ui::easing::out_cubic}, bool animate_scale = true
    );

    void show();
    void hide(bool scale = true);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    ui::TransitionSpec m_transition;
    bool m_animate_scale;
};
