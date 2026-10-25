#pragma once

#include "game/ui/controls.hpp"

class DashLevel;
class DashIconButton;

namespace ui {
    class Container;
    class TextWidget;
} // namespace ui

class LevelCard : public DashButton {
public:
    explicit LevelCard(const DashLevel& level);
    void on_remove(std::function<void()> callback);
    void set_removable(bool removable);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void on_update(float frametime) override;
    void draw_children() override;

private:
    const DashLevel& m_level;
    ui::Container* m_content = nullptr;
    ui::TextWidget* m_name = nullptr;
    ui::TextWidget* m_description = nullptr;
    DashIconButton* m_remove_button = nullptr;
};
