#pragma once

#include "../modal.hpp"

struct UI;

struct LevelSelectorModal : public UIModal {
    explicit LevelSelectorModal(UI* ui);

    void render() override;
};
