#pragma once

#include <imgui-ui/widgets/button.hpp>

class EditorTabButton : public ui::ButtonWidget {
public:
    explicit EditorTabButton(std::string name);

    void set_selected(bool value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void update_style();

    bool m_selected = false;
};
