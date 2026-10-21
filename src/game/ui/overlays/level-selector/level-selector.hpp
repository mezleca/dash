#pragma once

#include "../overlay.hpp"

#include <functional>

class Dash;
class DashLevel;

class LevelSelectorOverlay : public Overlay {
public:
    explicit LevelSelectorOverlay(Dash& game, std::string id = "default", std::function<void(DashLevel&)> on_select = {});

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
