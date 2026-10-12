#pragma once

#include "../option-overlay.hpp"

#include <functional>

class Dash;
class DashLevel;

class LevelSelectorOverlay : public MenuOptionOverlay {
public:
    explicit LevelSelectorOverlay(Dash& game, std::string id = "default", std::function<void(DashLevel&)> on_select = {});
};
