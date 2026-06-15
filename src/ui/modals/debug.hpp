#pragma once

#include "../modal.hpp"

struct UI;

struct DebugModal : public UIModal {
    explicit DebugModal(UI* ui);

    void render() override;
};
