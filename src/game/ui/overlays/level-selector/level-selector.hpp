#pragma once

#include "../overlay.hpp"

#include <functional>

class Dash;
class DashLevel;
class CarouselContainer;
class DashButton;
class DashDialog;

namespace ui {
    class TextWidget;
}

class LevelSelectorOverlay : public Overlay {
public:
    explicit LevelSelectorOverlay(Dash& game, std::string id = "default", std::function<void(DashLevel&)> on_select = {});
    void set_editor_mode(bool editor);
    bool cancel_creation();

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void on_update(float frametime) override;

private:
    void add_level(DashLevel& level);
    void show_create_panel();
    void create_level();

    Dash& m_game;
    std::function<void(DashLevel&)> m_on_select;
    CarouselContainer* m_carousel = nullptr;
    ui::TextWidget* m_empty_label = nullptr;
    DashButton* m_create_card = nullptr;
    ui::LayerContainer* m_create_panel = nullptr;
    DashButton* m_confirm_button = nullptr;
    DashDialog* m_music_field = nullptr;
    ui::TextWidget* m_error_label = nullptr;
    std::string m_name;
    std::string m_creator;
    std::string m_description;
    bool m_editor_mode = false;
};
