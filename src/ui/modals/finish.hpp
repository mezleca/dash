#pragma once

#include "../modal.hpp"

struct UI;

struct FinishModal : public UIModal {
    explicit FinishModal(UI* ui);

    void render() override;
};
