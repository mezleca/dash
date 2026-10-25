#pragma once

#include <imgui-ui/layout/layer-container.hpp>

namespace ui {
    class TextWidget;
}
class Dash;

class GameplayHud : public ui::LayerContainer {
public:
    explicit GameplayHud(Dash& game);

protected:
    void on_update(float frametime) override;
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    ui::TextWidget* m_fps = nullptr;
};
