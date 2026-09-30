#pragma once

#include <imgui-ui/widgets/button.hpp>

class MenuButton : public ui::ButtonWidget {
public:
    explicit MenuButton(std::string text);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
