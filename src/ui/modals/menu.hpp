#pragma once

#include "../modal.hpp"
#include <raylib.h>

struct UI;

struct MenuModal : public UIModal {
    explicit MenuModal(UI* ui);
    ~MenuModal() override;

    Texture2D m_logo_texture = {};

    void on_escape() override;
    void on_remove() override;
    void render() override;
};
