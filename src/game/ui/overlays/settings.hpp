#pragma once

#include "option-layer.hpp"

class Dash;

class SettingsLayer : public MenuOptionLayer {
public:
    explicit SettingsLayer(Dash& game, std::string id = "default");

protected:
    ImGuiWindowFlags child_window_flags() const override;

private:
    Dash& m_game;
    int m_volume = 100;
    bool m_godmode = false;
    bool m_free_mode = false;
};
