#pragma once

#include "option-layer.hpp"

#include <functional>

class ExitLayer : public MenuOptionLayer {
public:
    ExitLayer(std::string id, std::function<void()> on_cancel);
};
