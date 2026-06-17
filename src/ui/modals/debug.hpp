#pragma once

#include "../modal.hpp"

struct UI;

struct DebugModal : public UIModal {
    explicit DebugModal(UI* ui);

    void on_escape() override;
    void on_remove() override;
    void render() override;
};
