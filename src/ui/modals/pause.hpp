#pragma once

#include "../modal.hpp"

struct UI;

struct PauseModal : public UIModal {
    explicit PauseModal(UI* ui);

    void on_remove() override;
    void render() override;
};
