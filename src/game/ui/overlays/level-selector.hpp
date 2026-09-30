#pragma once

#include "option-layer.hpp"

#include <functional>

struct DashLevel;

class LevelSelectorLayer : public MenuOptionLayer {
public:
    explicit LevelSelectorLayer(std::string id = "default", std::function<void(DashLevel&)> on_select = {});
};
