#pragma once

#include "../overlay.hpp"

#include <functional>

class ExitOverlay : public Overlay {
public:
    ExitOverlay(std::string id, std::string message, std::function<void()> on_confirm, std::function<void()> on_cancel);
};
