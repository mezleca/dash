#pragma once

#include <imgui-ui/widgets/image.hpp>
#include "game/ui/controls.hpp"
#include <imgui-ui/widgets/text-input.hpp>

#include <functional>
#include <utility>

class EditorModeButton : public DashIconButton {
public:
    EditorModeButton(ui::Texture* icon, bool selected);
    void set_selected(bool value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void update_style(const ui::Theme& theme);

    bool m_selected = false;
};

class EditorTabButton : public DashButton {
public:
    explicit EditorTabButton(std::string name, bool selected = false);
    void set_selected(bool value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void update_style();

    bool m_selected = false;
};
