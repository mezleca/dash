#pragma once

#include "../modal.hpp"

struct UI;

struct LevelSelectorModal : public UIModal {
    explicit LevelSelectorModal(UI* ui);

    void on_remove() override;
    void on_escape() override;
    void render() override;
};
