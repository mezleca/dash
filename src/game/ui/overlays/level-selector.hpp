#pragma once

#include "option-layer.hpp"

#include <functional>

class DashLevel;

class LevelSelectorLayer : public MenuOptionLayer {
public:
    explicit LevelSelectorLayer(std::string id = "default", std::function<void(DashLevel&)> on_select = {});
};
