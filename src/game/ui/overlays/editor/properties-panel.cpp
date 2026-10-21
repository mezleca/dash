#include "properties-panel.hpp"

#include "core/components/background-sprite.hpp"
#include "core/components/sprite.hpp"
#include "game/dash.hpp"
#include "game/editor/editor.hpp"
#include "game/ui/controls.hpp"
#include "game/ui/widgets/dash-number-input.hpp"
#include "controls.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/color-picker.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/layout/tree-container.hpp>

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
    m_properties_tab->on_click([this] { select_tab(PropertiesTab::Properties); });

    m_metadata_tab = &tab_bar.add<EditorTabButton>("metadata");
    m_metadata_tab->on_click([this] { select_tab(PropertiesTab::Metadata); });

    m_body = &add<Container>("editor-properties-body");
    m_body->set_size({grow(), grow()});
    m_body->set_scrollable(true);
    m_body->set_spacing(6.0F);
    m_body->configure_all_styles([](Style& style) { style.padding({6.0F, 0.0F}); });
}

void PropertiesPanel::on_update(float) {
    LevelEditor* editor = m_game.editor();
    Entity* selected = editor != nullptr ? editor->selected_object() : nullptr;
    const bool multiple = editor != nullptr && editor->selected_objects().size() > 1;
    const bool changed = !m_initialized || selected != m_selected || multiple != m_multiple;
    m_selected = selected;
    m_multiple = multiple;
    if (changed && (!m_initialized || selected != nullptr || multiple || m_current_tab == PropertiesTab::Properties)) {
        select_tab(PropertiesTab::Properties);
    }
    m_initialized = true;

    // inputs apply edits immediately, syncing these values also catches the final frame of a drag
    const auto* body = selected != nullptr ? selected->get_component<RigidBody>() : nullptr;
    if (m_current_tab != PropertiesTab::Properties || body == nullptr) {
        return;
    }
    m_position = body->get_position();
    const Vector2 size = body->get_dimensions();
    if (m_size_label != nullptr && (size.x != m_size.x || size.y != m_size.y)) {
        m_size = size;
        m_size_label->set_text(TextFormat("size: %.1f x %.1f", size.x, size.y));
    }
}

DashNumberInput<float>& PropertiesPanel::add_number(Node& body, float& value, std::string label) {
    auto& field = body.add<DashNumberInput<float>>(value);
    field.set_label(std::move(label), LabelPlacement::Above);
    field.set_size({grow(), fit()});
    field.clear_range();
    return field;
}

void PropertiesPanel::select_tab(PropertiesTab tab) {
    m_current_tab = tab;
    m_properties_tab->set_selected(tab == PropertiesTab::Properties);
    m_metadata_tab->set_selected(tab == PropertiesTab::Metadata);

    if (tab == PropertiesTab::Properties) {
        rebuild();
    } else {
        m_size_label = nullptr;
        m_music_field = nullptr;
        m_body->clear();
        build_metadata(*m_body);
    }
}

void PropertiesPanel::rebuild() {
    m_size_label = nullptr;
    m_music_field = nullptr;
    m_body->clear();

    if (m_multiple) {
        auto& cock = m_body->add<TextWidget>("there is more than 1 GameObject selected buddy");
        cock.set_wrap(200.0f);
        cock.set_overflow(TextOverflow::Ellipsis);
        return;
    }

    Entity* selected = m_selected;
    if (selected == nullptr) {
        m_body->add<TextWidget>("select an object in the world");
        return;
    }

    auto& object = m_body->add<TreeContainer>("Object", "object-properties");
    object.set_spacing(6.0F);
    build_object(object, *selected);

    if (auto* rigid_body = selected->get_component<RigidBody>()) {
        auto& section = m_body->add<TreeContainer>("Rigid body", "rigid-body-properties");
        section.set_spacing(6.0F);
        build_rigid_body(section, *rigid_body);
    }

    if (auto* sprite = selected->get_component<Sprite>()) {
        auto& section = m_body->add<TreeContainer>("Sprite", "sprite-properties");
        section.set_spacing(6.0F);
        build_sprite(section, *sprite);
    }
}

void PropertiesPanel::build_object(Container& body, Entity& selected) {
    body.add<TextWidget>(TextFormat("type: %s", selected.type_to_str()));
    m_z_index = selected.z_order();
    auto& z_index = body.add<DashNumberInput<int>>(m_z_index);
    z_index.set_label("z index", LabelPlacement::Above);
    z_index.set_size({grow(), fit()});
    z_index.clear_range();
    z_index.on_change([this, object = &selected] {
        object->set_z_order(m_z_index);
        mark_changed();
    });

    m_visible = selected.is_visible();
    body.add<DashCheckbox>(m_visible, "visible").on_change([this, object = &selected] {
        object->set_visible(m_visible);
        mark_changed();
    });
}

void PropertiesPanel::build_rigid_body(Container& body, RigidBody& rigid_body) {
    m_position = rigid_body.get_position();
    add_number(body, m_position.x, "position x").on_change([this, &rigid_body] {
        rigid_body.set_position(m_position.x, m_position.y);
        mark_changed();
    });
    add_number(body, m_position.y, "position y").on_change([this, &rigid_body] {
        rigid_body.set_position(m_position.x, m_position.y);
        mark_changed();
    });

    m_size = rigid_body.get_dimensions();
    m_size_label = &body.add<TextWidget>(TextFormat("size: %.1f x %.1f", m_size.x, m_size.y));

    m_velocity = rigid_body.get_velocity();
    add_number(body, m_velocity.x, "velocity x").on_change([this, &rigid_body] {
        rigid_body.set_velocity(m_velocity.x, m_velocity.y);
        mark_changed();
    });

    add_number(body, m_velocity.y, "velocity y").on_change([this, &rigid_body] {
        rigid_body.set_velocity(m_velocity.x, m_velocity.y);
        mark_changed();
    });

    m_gravity = rigid_body.get_gravity();
    add_number(body, m_gravity, "gravity").on_change([this, &rigid_body] {
        rigid_body.set_gravity(m_gravity);
        mark_changed();
    });

    m_sensor = rigid_body.is_sensor();
    body.add<DashCheckbox>(m_sensor, "sensor").on_change([this, &rigid_body] {
        rigid_body.set_sensor(m_sensor);
        mark_changed();
    });

    m_collision_enabled = rigid_body.collision_enabled();
    body.add<DashCheckbox>(m_collision_enabled, "collision enabled").on_change([this, &rigid_body] {
        rigid_body.set_collision_enabled(m_collision_enabled);
        mark_changed();
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

    auto& body_type = body.add<DashDropdown>(
        m_body_type, std::vector<DropdownOption>{{"static", "static"}, {"kinematic", "kinematic"}, {"dynamic", "dynamic"}},
        "body type"
    );

    body_type.set_size({grow(), fit()});
    body_type.on_change([this, &rigid_body] {
        const b2BodyType type = m_body_type == "dynamic"     ? b2_dynamicBody
                                : m_body_type == "kinematic" ? b2_kinematicBody
                                                             : b2_staticBody;
        rigid_body.set_body_type(type);
        mark_changed();
    });
}

void PropertiesPanel::build_sprite(Container& body, Sprite& sprite) {
    m_rotation = sprite.rotation;
    add_number(body, m_rotation, "rotation").on_change([this, &sprite] {
        sprite.rotation = m_rotation;
        mark_changed();

        if (auto* body = sprite.object().get_component<RigidBody>(); body != nullptr && body->get_body_type() != b2_dynamicBody) {
            body->set_rotation(m_rotation);
        }
    });

    m_flip_x = sprite.flip_x;
    body.add<DashCheckbox>(m_flip_x, "flip x").on_change([this, &sprite] {
        sprite.flip_x = m_flip_x;
        mark_changed();
    });

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

    auto& sprite_type = body.add<DashDropdown>(
        m_sprite_type, std::vector<DropdownOption>{{"square", "square"}, {"triangle", "triangle"}, {"circle", "circle"}}, "shape"
    );

    sprite_type.set_size({grow(), fit()});
    sprite_type.on_change([this, &sprite] {
        sprite.type = m_sprite_type == "triangle" ? SpriteType::TRIANGLE
                      : m_sprite_type == "circle" ? SpriteType::CIRCLE
                                                  : SpriteType::SQUARE;
        mark_changed();
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
        mark_changed();
    });

    if (auto* background = dynamic_cast<BackgroundSprite*>(&sprite)) {
        m_fill_viewport = background->fill_viewport();
        body.add<DashCheckbox>(m_fill_viewport, "fill viewport").on_change([this, background] {
            background->set_fill_viewport(m_fill_viewport);
            mark_changed();
        });
    }
}

void PropertiesPanel::build_metadata(Container& body) {
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

    auto* name_field = &body.add<EditorTextInput>(m_name);
    name_field->set_label("name", LabelPlacement::Above);
    name_field->set_size({grow(), fit()});

    auto* creator_field = &body.add<EditorTextInput>(m_creator);
    creator_field->set_label("creator", LabelPlacement::Above);
    creator_field->set_size({grow(), fit()});

    auto* description_field = &body.add<EditorTextInput>(m_description);
    description_field->set_label("description", LabelPlacement::Above);
    description_field->set_size({grow(), fit()});

    auto* music_location_field = &body.add<DashDialog>("music");
    m_music_field = music_location_field;
    music_location_field->set_value(fs::path(m_music_location).filename().string());
    music_location_field->on_select([this, music_location_field](const FileDialogResult&) {
        m_music_location = music_location_field->value();
        music_location_field->set_value(fs::path(m_music_location).filename().string());
    });
}

bool PropertiesPanel::has_draft_changes() const {
    const auto* level = m_game.current_level();
    if (level == nullptr || level != m_level) {
        return false;
    }

    const auto& metadata = level->metadata();
    return m_name != metadata.name || m_creator != metadata.creator || m_description != metadata.description ||
           fs::path(m_music_location) != metadata.music.path();
}

void PropertiesPanel::mark_changed() {
    if (auto* editor = m_game.editor()) {
        editor->mark_changed();
    }
}

void PropertiesPanel::reset() {
    m_level = nullptr;
    m_initialized = false;
    m_selected = nullptr;
    m_multiple = false;
    m_size_label = nullptr;
    m_music_field = nullptr;
    m_body->clear();
}

bool PropertiesPanel::save() {
    auto* level = m_game.current_level();
    if (level == nullptr) {
        return false;
    }

    if (level == m_level) {
        auto& metadata = level->metadata();
        if (!m_music_location.empty() && fs::path(m_music_location) != metadata.music.path() &&
            !metadata.music.update(m_music_location)) {
            return false;
        }

        m_music_location = metadata.music.path().string();
        metadata.name = m_name;
        metadata.creator = m_creator;
        metadata.description = m_description;
        if (m_music_field != nullptr) {
            m_music_field->set_value(fs::path(m_music_location).filename().string());
        }
    }

    mark_changed();
    if (!level->save()) {
        return false;
    }

    if (auto* editor = m_game.editor()) {
        editor->mark_saved();
    }

    return true;
}
