#pragma once

#include "../modal.hpp"

struct UI;

struct DeathModal : public UIModal {
    explicit DeathModal(UI* ui);

    void render() override;
};
