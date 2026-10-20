#pragma once

#include <imgui-ui/widgets/button.hpp>

class DashButton : public ui::ButtonWidget {
public:
    explicit DashButton(std::string name);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
