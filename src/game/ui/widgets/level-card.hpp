#pragma once

#include <imgui-ui/widgets/button.hpp>

class DashLevel;

class LevelCard : public ui::ButtonWidget {
public:
    explicit LevelCard(const DashLevel& level);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
