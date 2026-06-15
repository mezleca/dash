#pragma once

#include "../modal.hpp"

struct UI;

struct PlayerModal : public UIModal {
    explicit PlayerModal(UI* ui);

    void render() override;
};
