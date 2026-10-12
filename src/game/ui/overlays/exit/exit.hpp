#pragma once

#include "../option-overlay.hpp"

#include <functional>

class ExitOverlay : public MenuOptionOverlay {
public:
    ExitOverlay(std::string id, std::function<void()> on_cancel);
};
