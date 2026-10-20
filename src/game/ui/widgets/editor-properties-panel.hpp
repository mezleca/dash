#pragma once

#include "dash-number-input.hpp"
#include <imgui-ui/layout/resizable-container.hpp>
#include <imgui-ui/style/color.hpp>
#include <raylib.h>

#include <cstddef>
#include <limits>
#include <string>

namespace ui {
    class Container;
} // namespace ui

class Dash;
class DashLevel;
class Entity;
class EditorTabButton;
class LevelEditor;
class RigidBody;
class Sprite;

class PropertiesPanel : public ui::ResizableContainer {
public:
    explicit PropertiesPanel(Dash& game);

protected:
    void on_update(float frametime) override;

private:
    enum class Tab {
        Properties,
        Metadata,
    };

    static DashNumberInput<float>& add_number(ui::Node& body, float& value, std::string label);

    void select_tab(Tab tab);
    void rebuild();
    void build_properties(ui::Container& body);
    void build_metadata(ui::Container& body);
    void build_object(ui::Container& body, Entity& selected);
    void build_rigid_body(ui::Container& body, RigidBody& rigid_body);
    void build_sprite(ui::Container& body, Sprite& sprite);
    void update_metadata();

    Dash& m_game;
    LevelEditor* m_editor = nullptr;
    std::size_t m_selection_version = std::numeric_limits<std::size_t>::max();
    ui::Container* m_body = nullptr;
    EditorTabButton* m_properties_tab = nullptr;
    EditorTabButton* m_metadata_tab = nullptr;
    Tab m_current_tab = Tab::Properties;
    bool m_initialized = false;

    DashLevel* m_level = nullptr;
    std::string m_name;
    std::string m_creator;
    std::string m_description;
    std::string m_music_location;

    int m_z_index = 0;
    bool m_visible = true;
    Vector2 m_position = {};
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
