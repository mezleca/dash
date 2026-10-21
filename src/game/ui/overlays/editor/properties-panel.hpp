#pragma once

#include "game/ui/widgets/dash-number-input.hpp"
#include <imgui-ui/layout/resizable-container.hpp>
#include <imgui-ui/style/color.hpp>
#include <raylib.h>

#include <string>

namespace ui {
    class Container;
    class TextWidget;
} // namespace ui

class Dash;
class DashDialog;
class DashLevel;
class Entity;
class EditorTabButton;
class RigidBody;
class Sprite;

enum class PropertiesTab {
    Properties,
    Metadata,
};

class PropertiesPanel : public ui::ResizableContainer {
public:
    explicit PropertiesPanel(Dash& game);
    bool has_draft_changes() const;
    bool save();
    void reset();

protected:
    void on_update(float frametime) override;

private:
    static DashNumberInput<float>& add_number(ui::Node& body, float& value, std::string label);

    void select_tab(PropertiesTab tab);
    void rebuild();
    void build_metadata(ui::Container& body);
    void build_object(ui::Container& body, Entity& selected);
    void build_rigid_body(ui::Container& body, RigidBody& rigid_body);
    void build_sprite(ui::Container& body, Sprite& sprite);
    void mark_changed();

    Dash& m_game;
    Entity* m_selected = nullptr;
    ui::Container* m_body = nullptr;
    ui::TextWidget* m_size_label = nullptr;
    DashDialog* m_music_field = nullptr;
    EditorTabButton* m_properties_tab = nullptr;
    EditorTabButton* m_metadata_tab = nullptr;
    PropertiesTab m_current_tab = PropertiesTab::Properties;
    bool m_initialized = false;
    bool m_multiple = false;

    DashLevel* m_level = nullptr;
    std::string m_name;
    std::string m_creator;
    std::string m_description;
    std::string m_music_location;

    int m_z_index = 0;
    bool m_visible = true;
    Vector2 m_position = {};
    Vector2 m_size = {};
    Vector2 m_velocity = {};
    float m_gravity = 0.0F;
    bool m_sensor = false;
    bool m_collision_enabled = true;
    std::string m_body_type;
    float m_rotation = 0.0F;
    bool m_flip_x = false;
    std::string m_sprite_type;
    ui::Color m_tint;
    bool m_fill_viewport = false;
};
