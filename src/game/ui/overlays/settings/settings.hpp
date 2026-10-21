#pragma once

#include "../overlay.hpp"

class Dash;

class SettingsOverlay : public Overlay {
public:
    explicit SettingsOverlay(Dash& game, std::string id = "default");

private:
    Dash& m_game;
    int m_volume = 100;
    bool m_godmode = false;
    bool m_free_mode = false;
};
