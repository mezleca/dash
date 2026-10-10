#pragma once

#include "option-layer.hpp"

#include <functional>

class Dash;
class DashLevel;

class LevelSelectorLayer : public MenuOptionLayer {
public:
    explicit LevelSelectorLayer(Dash& game, std::string id = "default", std::function<void(DashLevel&)> on_select = {});
};
