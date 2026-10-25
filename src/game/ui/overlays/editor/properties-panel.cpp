#include "properties-panel.hpp"

#include "game/objects/world/image-sprite.hpp"
#include "game/objects/world/solid.hpp"
#include "game/objects/world/text.hpp"
#include "core/components/sprite.hpp"
#include "core/components/particles.hpp"
#include "game/dash.hpp"
#include "game/editor/editor.hpp"
#include "game/ui/controls.hpp"
#include "game/ui/theme.hpp"
#include "game/ui/widgets/dash-number-input.hpp"
#include "game/ui/widgets/dash-vector-input.hpp"
#include "controls.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/layout/tree-container.hpp>

using namespace ui;

static ::Color raylib_color(const ui::Color& value) {
    const ImVec4 color = value.rgba();
    const auto channel = [](float component) {
        return static_cast<unsigned char>(std::lround(std::clamp(component, 0.0F, 1.0F) * 255.0F));
    };

    return {channel(color.x), channel(color.y), channel(color.z), channel(color.w)};
}

template <class T>
static T* common_component(const std::vector<Entity*>& selection) {
    if (selection.empty()) {
        return nullptr;
    }

    for (const auto* object : selection) {
        if (object->get_component<T>() == nullptr) {
            return nullptr;
        }
    }

    return selection.front()->get_component<T>();
}

PropertiesPanel::PropertiesPanel(Dash& game, std::function<void(Entity&, Behaviour*)> open_behaviour_dialog)
    : ResizableContainer("editor-properties"), m_game(game), m_open_behaviour_dialog(std::move(open_behaviour_dialog)) {
    set_size({px(260.0F), grow()});
    set_resize(ResizeAxes::X);
    set_direction(StackDirection::Vertical);
    set_spacing(4.0F);

    auto& tab_bar = add<Container>("editor-properties-tabs", StackDirection::Horizontal);
    tab_bar.set_size({grow(), fit()});
    tab_bar.configure_all_styles([](Style& style) {
        style.border_radius(0.0F);
        style.border_bottom(1.0F);
        style.border_color(rgba(149, 204, 221, 60));
    });
    tab_bar.set_spacing(12.0F);
    tab_bar.set_input_mode(InputMode::None);

    m_properties_tab = &tab_bar.add<EditorTabButton>("properties");
    m_properties_tab->on_click([this] { select_tab(PropertiesTab::Properties); });

    m_metadata_tab = &tab_bar.add<EditorTabButton>("metadata");
    m_metadata_tab->on_click([this] { select_tab(PropertiesTab::Metadata); });

    m_body = &add<Container>("editor-properties-body");
    m_body->set_size({grow(), grow()});
    m_body->set_scrollable(true);
    m_body->set_spacing(4.0F);
    m_body->configure_all_styles([](Style& style) { style.padding({10.0F, 4.0F}); });
}

void PropertiesPanel::apply_theme_defaults(const Theme& theme) {
    ResizableContainer::apply_theme_defaults(theme);
    configure_all_styles([&theme](Style& style) { dash_ui::editor_panel_style(style, theme); });
}

void PropertiesPanel::reset() {
    m_level = nullptr;
    m_initialized = false;
    m_selected = nullptr;
    m_selection.clear();
    m_multiple = false;
    m_music_field = nullptr;
    m_behaviour_list = nullptr;
    m_body->clear();
}

void PropertiesPanel::refresh() {
    m_initialized = false;
}

void PropertiesPanel::on_update(float) {
    LevelEditor* editor = m_game.editor();
    const std::vector<Entity*> empty_selection;
    const auto& selection = editor != nullptr ? editor->selected_objects() : empty_selection;
    Entity* selected = selection.empty() ? nullptr : selection.front();

    const bool multiple = selection.size() > 1;
    const bool changed = !m_initialized || selection != m_selection;

    if (changed) {
        m_selection = selection;
    }

    m_selected = selected;
    m_multiple = multiple;

    if (changed && (!m_initialized || selected != nullptr || m_current_tab == PropertiesTab::Properties)) {
        select_tab(PropertiesTab::Properties);
    }

    m_initialized = true;

    // sync the final drag position.
    const auto* body = selected != nullptr ? selected->get_component<RigidBody>() : nullptr;

    if (m_current_tab != PropertiesTab::Properties || body == nullptr) {
        return;
    }

    m_position = body->get_position();
    m_size = body->get_size();
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
    m_music_field = nullptr;
    m_behaviour_list = nullptr;
    m_body->clear();

    if (tab == PropertiesTab::Properties) {
        rebuild();
    } else {
        build_metadata(*m_body);
    }
}

void PropertiesPanel::rebuild() {
    Entity* selected = m_selected;
    if (selected == nullptr) {
        m_body->add<TextWidget>("select an object in the world");
        return;
    }

    const auto add_section = [this](const char* label, const char* id) -> TreeContainer& {
        auto& section = m_body->add<TreeContainer>(label, id);
        section.set_spacing(4.0F);
        const auto color = surface().theme().text_color;
        section.configure_all_styles([color](Style& style) { style.color(color); });
        return section;
    };

    auto& object = add_section("Object", "object-properties");
    build_object(object, *selected);

    if (auto* rigid_body = common_component<RigidBody>(m_selection)) {
        auto& section = add_section("Rigid body", "rigid-body-properties");
        build_rigid_body(section, *rigid_body);
    }

    if (auto* sprite = common_component<Sprite>(m_selection)) {
        auto& section = add_section("Sprite", "sprite-properties");
        build_sprite(section, *sprite);
    }

    if (auto* particles = common_component<Particles>(m_selection)) {
        auto& section = add_section("Particles", "particle-properties");
        build_particles(section, *particles);
    }

    if (auto* text = common_component<TextSprite>(m_selection)) {
        auto& section = add_section("Text", "text-properties");
        m_text = text->text;
        auto& content = section.add<DashTextInput>(m_text, "text", LabelPlacement::Above);
        content.on_change([this] {
            edit_objects([this](Entity& object) { object.get_component<TextSprite>()->text = m_text; });
        });
    }

    auto& behaviours = add_section("Behaviours", "object-behaviours");
    build_behaviours(behaviours, *selected);

    auto& remove = m_body->add<DashDeleteButton>();
    remove.configure_all_styles([](Style& style) { style.margin_top(5.0F); });
    remove.on_click([this] {
        if (auto* editor = m_game.editor()) editor->delete_selection();
    });
}

void PropertiesPanel::build_object(Container& body, Entity& selected) {
    const bool common_type = std::all_of(m_selection.begin(), m_selection.end(), [&selected](const Entity* entity) {
        return entity->type() == selected.type();
    });

    body.add<TextWidget>(TextFormat("type: %s", common_type ? selected.type_to_str() : "multiple"));
    if (m_multiple) {
        body.add<TextWidget>("values shown from first selected object");
    }

    m_z_index = selected.z_order();
    auto& z_index = body.add<DashNumberInput<int>>(m_z_index);
    z_index.set_label("z index", LabelPlacement::Above);
    z_index.set_size({grow(), fit()});
    z_index.clear_range();
    z_index.on_change([this] { edit_objects([this](Entity& object) { object.set_z_order(m_z_index); }); });

    m_visible = selected.is_visible();
    body.add<DashCheckbox>(m_visible, "visible").on_change([this] {
        edit_objects([this](Entity& object) { object.set_visible(m_visible); });
    });
}

void PropertiesPanel::build_rigid_body(Container& body, RigidBody& rigid_body) {
    m_position = rigid_body.get_position();
    auto& position = body.add<DashVectorInput>(m_position, "position");

    // preserve other axes across multi-selection edits.
    position.component(VectorAxis::X).on_change([this] {
        edit_objects([this](Entity& object) {
            auto& rigid_body = *object.get_component<RigidBody>();
            rigid_body.set_position(m_position.x, rigid_body.get_position().y);
        });
    });

    position.component(VectorAxis::Y).on_change([this] {
        edit_objects([this](Entity& object) {
            auto& rigid_body = *object.get_component<RigidBody>();
            rigid_body.set_position(rigid_body.get_position().x, m_position.y);
        });
    });

    m_size = rigid_body.get_size();
    auto& size = body.add<DashVectorInput>(m_size, "size");

    size.set_minimum(1.0F);
    size.component(VectorAxis::X).on_change([this] {
        edit_objects([this](Entity& object) {
            auto& rigid_body = *object.get_component<RigidBody>();
            rigid_body.set_size({m_size.x, rigid_body.get_size().y});
        });
    });

    size.component(VectorAxis::Y).on_change([this] {
        edit_objects([this](Entity& object) {
            auto& rigid_body = *object.get_component<RigidBody>();
            rigid_body.set_size({rigid_body.get_size().x, m_size.y});
        });
    });

    m_velocity = rigid_body.get_velocity();
    auto& velocity = body.add<DashVectorInput>(m_velocity, "velocity");

    velocity.component(VectorAxis::X).on_change([this] {
        edit_objects([this](Entity& object) {
            auto& rigid_body = *object.get_component<RigidBody>();
            rigid_body.set_velocity(m_velocity.x, rigid_body.get_velocity().y);
        });
    });

    velocity.component(VectorAxis::Y).on_change([this] {
        edit_objects([this](Entity& object) {
            auto& rigid_body = *object.get_component<RigidBody>();
            rigid_body.set_velocity(rigid_body.get_velocity().x, m_velocity.y);
        });
    });

    m_gravity = rigid_body.get_gravity();
    add_number(body, m_gravity, "gravity").on_change([this] {
        edit_objects([this](Entity& object) { object.get_component<RigidBody>()->set_gravity(m_gravity); });
    });

    m_sensor = rigid_body.is_sensor();
    body.add<DashCheckbox>(m_sensor, "sensor").on_change([this] {
        edit_objects([this](Entity& object) { object.get_component<RigidBody>()->set_sensor(m_sensor); });
    });

    m_collision_enabled = rigid_body.collision_enabled();
    body.add<DashCheckbox>(m_collision_enabled, "collision enabled").on_change([this] {
        edit_objects([this](Entity& object) { object.get_component<RigidBody>()->set_collision_enabled(m_collision_enabled); });
    });

    m_body_type = RigidBody::type_to_str(rigid_body.get_body_type());

    auto& body_type = body.add<DashDropdown>(
        m_body_type, std::vector<DropdownOption>{{"static", "static"}, {"kinematic", "kinematic"}, {"dynamic", "dynamic"}},
        "body type"
    );

    body_type.set_size({grow(), fit()});
    body_type.on_change([this] {
        const b2BodyType type = RigidBody::type_from_str(m_body_type);
        edit_objects([type](Entity& object) { object.get_component<RigidBody>()->set_body_type(type); });
    });
}

void PropertiesPanel::build_sprite(Container& body, Sprite& sprite) {
    m_rotation = sprite.rotation;
    add_number(body, m_rotation, "rotation").on_change([this] {
        edit_objects([this](Entity& object) {
            object.get_component<Sprite>()->rotation = m_rotation;
            if (auto* body = object.get_component<RigidBody>(); body != nullptr && body->get_body_type() != b2_dynamicBody) {
                body->set_rotation(m_rotation);
            }
        });
    });

    m_flip_x = sprite.flip_x;
    body.add<DashCheckbox>(m_flip_x, "flip x").on_change([this] {
        edit_objects([this](Entity& object) { object.get_component<Sprite>()->flip_x = m_flip_x; });
    });

    if (common_component<TextSprite>(m_selection) == nullptr) {
        m_sprite_type = Sprite::type_to_str(sprite.type);

        auto& sprite_type = body.add<DashDropdown>(
            m_sprite_type, std::vector<DropdownOption>{{"square", "square"}, {"triangle", "triangle"}, {"circle", "circle"}},
            "shape"
        );

        sprite_type.set_size({grow(), fit()});
        sprite_type.on_change([this] {
            const auto type = Sprite::type_from_str(m_sprite_type);

            edit_objects([type](Entity& object) {
                if (auto* solid = dynamic_cast<Solid*>(&object)) {
                    solid->set_shape(type);
                } else {
                    object.get_component<Sprite>()->type = type;
                }
            });

            refresh();
        });
    }

    m_tint = rgba(sprite.tint.r, sprite.tint.g, sprite.tint.b, sprite.tint.a);
    auto& tint = body.add<DashColorPicker>(m_tint, "tint");
    tint.set_size({grow(), fit()});
    tint.on_change([this] {
        const ::Color value = raylib_color(m_tint);
        edit_objects([value](Entity& object) { object.get_component<Sprite>()->tint = value; });
    });

    if (auto* image = common_component<ImageSprite>(m_selection)) {
        auto& texture = body.add<DashDialog>("image");
        texture.set_filters({{"Images", "png,jpg,jpeg,bmp,gif,qoi,dds"}});
        texture.set_value(fs::path(image->texture_location()).filename().string());
        texture.on_select([this, &texture](const FileDialogResult& result) {
            if (result.paths.empty()) return;
            const fs::path path = fs::absolute(result.paths.front());
            edit_objects([&path](Entity& object) { object.get_component<ImageSprite>()->load_texture(path.string().c_str()); });
            auto* image = m_selected->get_component<ImageSprite>();
            texture.set_value(fs::path(image->texture_location()).filename().string());
        });
        m_image_space = image->space() == ImageSpace::Static ? "static" : "world";
        auto& space =
            body.add<DashDropdown>(m_image_space, std::vector<DropdownOption>{{"world", "world"}, {"static", "static"}}, "space");
        space.set_size({grow(), fit()});
        space.on_change([this] {
            edit_objects([this](Entity& object) {
                object.get_component<ImageSprite>()->set_space(
                    m_image_space == "static" ? ImageSpace::Static : ImageSpace::World, m_game.get_viewport(0)->camera().view(),
                    m_game.get_viewport(0)->size()
                );
            });
            refresh();
        });

        if (image->space() != ImageSpace::Static) {
            return;
        }

        m_fill_viewport = image->fill_viewport();
        body.add<DashCheckbox>(m_fill_viewport, "fill viewport").on_change([this] {
            edit_objects([this](Entity& object) { object.get_component<ImageSprite>()->set_fill_viewport(m_fill_viewport); });
        });
    }
}

void PropertiesPanel::build_particles(Container& body, Particles& particles) {
    if (m_multiple) {
        body.add<TextWidget>("select one emitter to edit particles");
        return;
    }

    const auto changed = [this] { mark_changed(); };
    const auto number = [&](float& value, const char* label) -> DashNumberInput<float>& {
        auto& field = add_number(body, value, label);
        field.on_change(changed);
        return field;
    };
    auto& prototype = particles.prototype;
    number(particles.emission_rate, "rate (particles/sec)").set_minimum(0.0F);
    number(prototype.radius, "radius").set_minimum(0.01F);
    number(prototype.lifetime, "lifetime (seconds)").set_minimum(0.01F);
    number(prototype.growth, "growth (px/sec)");
    body.add<DashVectorInput>(prototype.position, "offset").on_change(changed);
    body.add<DashVectorInput>(prototype.velocity, "velocity").on_change(changed);
    body.add<DashVectorInput>(prototype.acceleration, "acceleration").on_change(changed);

    const ::Color color = prototype.color;
    m_particle_tint = rgba(color.r, color.g, color.b, color.a);
    auto& tint = body.add<DashColorPicker>(m_particle_tint, "color");
    tint.set_size({grow(), fit()});
    tint.on_change([this, &particles] {
        particles.prototype.color = raylib_color(m_particle_tint);
        mark_changed();
    });
}

void PropertiesPanel::build_behaviours(Container& body, Entity& object) {
    if (m_multiple) {
        body.add<TextWidget>("select one object to edit behaviours");
        return;
    }

    body.add<DashButton>("add").on_click([this, &object] { m_open_behaviour_dialog(object, nullptr); });
    m_behaviour_list = &body.add<Container>("behaviour-list");
    m_behaviour_list->set_size({grow(), {fit(), px(0), px(180)}});
    m_behaviour_list->set_scrollable(true);
    m_behaviour_list->set_spacing(4.0F);
    refresh_behaviours();
}

void PropertiesPanel::refresh_behaviours() {
    if (m_behaviour_list == nullptr || m_selected == nullptr || m_multiple) return;
    m_behaviour_list->clear();
    for (const auto& behaviour : m_selected->behaviours()) {
        auto* entry = behaviour.get();
        m_behaviour_list->add<DashButton>(entry->name(), LayoutSize{grow(), fit()}).on_click([this, entry] {
            m_open_behaviour_dialog(*m_selected, entry);
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
        m_initial_type = std::to_string(static_cast<int>(metadata.initial_type));
        m_level = level;
    }

    body.add<DashTextInput>(m_name, "name", LabelPlacement::Above);
    body.add<DashTextInput>(m_creator, "creator", LabelPlacement::Above);
    body.add<DashTextInput>(m_description, "description", LabelPlacement::Above);

    auto& initial_type =
        body.add<DashDropdown>(m_initial_type, std::vector<DropdownOption>{{"box", "0"}, {"bird", "1"}}, "initial player type");
    initial_type.set_size({grow(), fit()});

    const ::Color background = level->solid_background();
    m_solid_background = rgba(background.r, background.g, background.b, background.a);

    auto& background_field = body.add<DashColorPicker>(m_solid_background, "solid background");

    background_field.set_size({grow(), fit()});
    background_field.on_change([this, level] {
        level->set_solid_background(raylib_color(m_solid_background));
        mark_changed();
    });

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
           fs::path(m_music_location) != metadata.music.path() ||
           m_initial_type != std::to_string(static_cast<int>(metadata.initial_type));
}

void PropertiesPanel::mark_changed() {
    if (auto* editor = m_game.editor()) {
        editor->mark_changed();
    }
}

void PropertiesPanel::edit_objects(const std::function<void(Entity&)>& edit) {
    for (auto* object : m_selection) {
        edit(*object);
    }
    mark_changed();
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
        metadata.initial_type = static_cast<PlayerType>(std::stoi(m_initial_type));
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
