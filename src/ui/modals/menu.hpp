#pragma once

#include "../modal.hpp"
#include <raylib.h>

struct UI;

struct MenuModal : public UIModal {
    explicit MenuModal(UI* ui);

    Texture2D m_logo_texture;

    void render() override;
};
