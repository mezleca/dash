#pragma once

#include "../modal.hpp"

struct UI;

enum class PlayfieldMode { NONE = 0, PAUSE, DEATH, FINISH };

struct PlayfieldModal : public UIModal {
    explicit PlayfieldModal(UI* ui);

    PlayfieldMode m_mode;

    void show_default_screen();
    void show_pause_screen();
    void show_death_screen();
    void show_finish_screen();

    void on_escape() override;
    void on_remove() override;
    void render() override;
};
