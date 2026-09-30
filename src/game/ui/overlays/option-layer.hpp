#pragma once

#include <imgui-ui/layout/layer-container.hpp>

class MenuOptionLayer : public ui::LayerContainer {
public:
    explicit MenuOptionLayer(std::string id);

    void show();
    void hide(bool scale = true);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
