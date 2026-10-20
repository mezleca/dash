#include "editor-properties-panel.hpp"

#include "core/components/background-sprite.hpp"
#include "core/components/sprite.hpp"
#include "game/dash.hpp"
#include "game/editor/editor.hpp"
#include "game/ui/widgets/dash-button.hpp"
#include "game/ui/widgets/dash-number-input.hpp"
#include "game/ui/widgets/dash-text-input.hpp"
#include "game/ui/widgets/editor-tab-button.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/checkbox.hpp>
#include <imgui-ui/widgets/color-picker.hpp>
#include <imgui-ui/widgets/dropdown.hpp>
#include <imgui-ui/widgets/file-dialog.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/layout/container.hpp>

using namespace ui;

PropertiesPanel::PropertiesPanel(Dash& game) : ResizableContainer("editor-properties"), m_game(game) {
    set_size({px(260.0F), grow()});
    set_resize(ResizeAxes::X);
    set_direction(StackDirection::Vertical);
    set_spacing(4.0F);

    configure_all_styles([](Style& style) {
        style.background_color(rgba(33, 34, 38, 240));
        style.border(BorderEdge{1.0F, BorderType::Solid});
        style.border_color(rgba(149, 204, 221, 60));
        style.border_radius(4.0F);
        style.padding({6.0F, 6.0F});
    });

    auto& tab_bar = add<Container>("editor-properties-tabs", StackDirection::Horizontal);
    tab_bar.set_size({grow(), fit()});
    tab_bar.set_spacing(2.0F);
    tab_bar.set_input_mode(InputMode::None);

    m_properties_tab = &tab_bar.add<EditorTabButton>("properties");
    m_properties_tab->on_click([this] { select_tab(Tab::Properties); });

    m_metadata_tab = &tab_bar.add<EditorTabButton>("metadata");
    m_metadata_tab->on_click([this] { select_tab(Tab::Metadata); });

    m_body = &add<Container>("editor-properties-body");
    m_body->set_size({grow(), grow()});
    m_body->set_scrollable(true);
    m_body->set_spacing(6.0F);
    m_body->configure_all_styles([](Style& style) { style.padding({6.0F, 0.0F}); });
}

void PropertiesPanel::on_update(float) {
    if (!m_initialized) {
        m_initialized = true;
        select_tab(Tab::Properties);
    }

    LevelEditor* editor = m_game.editor();
    if (editor != m_editor) {
        m_editor = editor;
        m_selection_version = std::numeric_limits<std::size_t>::max();
    }

    const std::size_t version = editor != nullptr ? editor->selection_version() : 0;
    if (version == m_selection_version) {
        return;
    }

    m_selection_version = version;
    if (editor != nullptr && editor->selected_object() != nullptr && m_current_tab != Tab::Properties) {
        select_tab(Tab::Properties);
        return;
    }

    if (m_current_tab == Tab::Properties) {
        rebuild();
    }
}

DashNumberInput<float>& PropertiesPanel::add_number(Node& body, float& value, std::string label) {
    auto& field = body.add<DashNumberInput<float>>(value);
    field.set_label(std::move(label), LabelPlacement::Above);
    field.set_size({grow(), fit()});
    field.clear_range();
    return field;
}

void PropertiesPanel::select_tab(Tab tab) {
    m_current_tab = tab;
    m_properties_tab->set_selected(tab == Tab::Properties);
    m_metadata_tab->set_selected(tab == Tab::Metadata);

    m_body->clear();
    if (tab == Tab::Properties) {
        rebuild();
    } else {
        build_metadata(*m_body);
    }
}

void PropertiesPanel::rebuild() {
    m_body->clear();
    m_body->add<TextWidget>("Properties").set_font(surface().get_primary_font(), 32.0F);

    Entity* selected = m_editor != nullptr ? m_editor->selected_object() : nullptr;
    if (selected == nullptr) {
        m_body->add<TextWidget>("select an object in the world");
        return;
    }

    build_object(*m_body, *selected);

    if (auto* rigid_body = selected->get_component<RigidBody>()) {
        build_rigid_body(*m_body, *rigid_body);
    }

    if (auto* sprite = selected->get_component<Sprite>()) {
        build_sprite(*m_body, *sprite);
    }
}

void PropertiesPanel::build_object(Container& body, Entity& selected) {
    body.add<TextWidget>(TextFormat("type: %s", selected.type_to_str()));
    m_z_index = selected.z_order();
    auto& z_index = body.add<DashNumberInput<int>>(m_z_index);
    z_index.set_label("z index", LabelPlacement::Above);
    z_index.set_size({grow(), fit()});
    z_index.clear_range();
    z_index.on_change([this, object = &selected] { object->set_z_order(m_z_index); });

    m_visible = selected.is_visible();
    body.add<CheckboxWidget>(m_visible, "visible").on_change([this, object = &selected] { object->set_visible(m_visible); });
}

void PropertiesPanel::build_rigid_body(Container& body, RigidBody& rigid_body) {
    body.add<TextWidget>("Rigid body").set_font(surface().get_primary_font(), 28.0F);

    m_position = rigid_body.get_position();
    add_number(body, m_position.x, "position x").on_change([this, &rigid_body] {
        rigid_body.set_position(m_position.x, m_position.y);
    });
    add_number(body, m_position.y, "position y").on_change([this, &rigid_body] {
        rigid_body.set_position(m_position.x, m_position.y);
    });

    const Rectangle bounds = rigid_body.get_bounding_box();
    body.add<TextWidget>(TextFormat("size: %.1f x %.1f", bounds.width, bounds.height));

    m_velocity = rigid_body.get_velocity();
    add_number(body, m_velocity.x, "velocity x").on_change([this, &rigid_body] {
        rigid_body.set_velocity(m_velocity.x, m_velocity.y);
    });
    add_number(body, m_velocity.y, "velocity y").on_change([this, &rigid_body] {
        rigid_body.set_velocity(m_velocity.x, m_velocity.y);
    });

    m_gravity = rigid_body.get_gravity();
    add_number(body, m_gravity, "gravity").on_change([this, &rigid_body] { rigid_body.set_gravity(m_gravity); });

    m_sensor = rigid_body.is_sensor();
    body.add<CheckboxWidget>(m_sensor, "sensor").on_change([this, &rigid_body] { rigid_body.set_sensor(m_sensor); });

    m_collision_enabled = rigid_body.collision_enabled();
    body.add<CheckboxWidget>(m_collision_enabled, "collision enabled").on_change([this, &rigid_body] {
        rigid_body.set_collision_enabled(m_collision_enabled);
    });

    switch (rigid_body.get_body_type()) {
        case b2_dynamicBody:
            m_body_type = "dynamic";
            break;
        case b2_kinematicBody:
            m_body_type = "kinematic";
            break;
        default:
            m_body_type = "static";
            break;
    }

    auto& body_type = body.add<DropdownWidget>(
        m_body_type, std::vector<DropdownOption>{{"static", "static"}, {"kinematic", "kinematic"}, {"dynamic", "dynamic"}},
        "body type"
    );
    body_type.set_size({grow(), fit()});
    body_type.on_change([this, &rigid_body] {
        const b2BodyType type = m_body_type == "dynamic"     ? b2_dynamicBody
                                : m_body_type == "kinematic" ? b2_kinematicBody
                                                             : b2_staticBody;
        rigid_body.set_body_type(type);
    });
}

void PropertiesPanel::build_sprite(Container& body, Sprite& sprite) {
    body.add<TextWidget>("Sprite").set_font(surface().get_primary_font(), 28.0F);

    m_rotation = sprite.rotation;
    add_number(body, m_rotation, "rotation").on_change([this, &sprite] {
        sprite.rotation = m_rotation;

        if (auto* body = sprite.object().get_component<RigidBody>(); body != nullptr && body->get_body_type() != b2_dynamicBody) {
            body->set_rotation(m_rotation);
        }
    });

    m_flip_x = sprite.flip_x;
    body.add<CheckboxWidget>(m_flip_x, "flip x").on_change([this, &sprite] { sprite.flip_x = m_flip_x; });

    switch (sprite.type) {
        case SpriteType::TRIANGLE:
            m_sprite_type = "triangle";
            break;
        case SpriteType::CIRCLE:
            m_sprite_type = "circle";
            break;
        default:
            m_sprite_type = "square";
            break;
    }

    auto& sprite_type = body.add<DropdownWidget>(
        m_sprite_type, std::vector<DropdownOption>{{"square", "square"}, {"triangle", "triangle"}, {"circle", "circle"}}, "shape"
    );
    sprite_type.set_size({grow(), fit()});
    sprite_type.on_change([this, &sprite] {
        sprite.type = m_sprite_type == "triangle" ? SpriteType::TRIANGLE
                      : m_sprite_type == "circle" ? SpriteType::CIRCLE
                                                  : SpriteType::SQUARE;
    });

    m_tint = rgba(sprite.tint.r, sprite.tint.g, sprite.tint.b, sprite.tint.a);
    auto& tint = body.add<ColorPickerWidget>(m_tint, "tint");
    tint.set_size({grow(), fit()});
    tint.on_change([this, &sprite] {
        const ImVec4 color = m_tint.rgba();
        const auto channel = [](float value) {
            return static_cast<unsigned char>(std::lround(std::clamp(value, 0.0F, 1.0F) * 255.0F));
        };
        sprite.tint = {channel(color.x), channel(color.y), channel(color.z), channel(color.w)};
    });

    if (auto* background = dynamic_cast<BackgroundSprite*>(&sprite)) {
        m_fill_viewport = background->fill_viewport();
        body.add<CheckboxWidget>(m_fill_viewport, "fill viewport").on_change([this, background] {
            background->set_fill_viewport(m_fill_viewport);
        });
    }
}

void PropertiesPanel::build_metadata(Container& body) {
    body.add<TextWidget>("Metadata").set_font(surface().get_primary_font(), 32.0F);

    auto* level = m_game.current_level();
    if (level == nullptr) {
        body.add<TextWidget>("no level loaded");
        return;
    }

    if (m_level != level) {
        const auto& metadata = level->metadata();
        m_name = metadata.name;
        m_creator = metadata.creator;
        m_description = metadata.description;
        m_music_location = metadata.music.path().string();
        m_level = level;
    }

    auto* name_field = &body.add<DashTextInput>(m_name);
    name_field->set_label("name", LabelPlacement::Above);
    name_field->set_size({grow(), fit()});

    auto* creator_field = &body.add<DashTextInput>(m_creator);
    creator_field->set_label("creator", LabelPlacement::Above);
    creator_field->set_size({grow(), fit()});

    auto* description_field = &body.add<DashTextInput>(m_description);
    description_field->set_label("description", LabelPlacement::Above);
    description_field->set_size({grow(), fit()});

    auto* music_location_field = &body.add<FileDialogWidget>("music", LabelPlacement::Above);
    music_location_field->set_value(fs::path(m_music_location).filename().string());
    music_location_field->on_select([this, music_location_field](const FileDialogResult&) {
        m_music_location = music_location_field->value();
        music_location_field->set_value(fs::path(m_music_location).filename().string());
    });

    auto& update = body.add<DashButton>("update");
    update.on_click([this, music_location_field] {
        update_metadata();
        music_location_field->set_value(fs::path(m_music_location).filename().string());
    });
}

void PropertiesPanel::update_metadata() {
    auto* level = m_game.current_level();
    if (level == nullptr || level != m_level) {
        return;
    }

    auto& metadata = level->metadata();
    if (!m_music_location.empty() && fs::path(m_music_location) != metadata.music.path() &&
        !metadata.music.update(m_music_location)) {
        return;
    }

    m_music_location = metadata.music.path().string();
    metadata.name = m_name;
    metadata.creator = m_creator;
    metadata.description = m_description;

    m_level->save();
}
