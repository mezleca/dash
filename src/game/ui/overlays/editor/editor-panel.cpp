#include "editor.hpp"
#include "game/dash.hpp"
#include "core/components/sprite.hpp"
#include "core/components/background-sprite.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/runtime.hpp>
#include <imgui-ui/widgets/button.hpp>
#include <imgui-ui/widgets/checkbox.hpp>
#include <imgui-ui/widgets/color-picker.hpp>
#include <imgui-ui/widgets/dropdown.hpp>
#include <imgui-ui/widgets/number-input.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/widgets/text-input.hpp>
#include <imgui-ui/widgets/file-dialog.hpp>
#include <imgui-ui/widgets/image.hpp>
#include <imgui-ui/layout/container.hpp>

using namespace ui;

enum class EditorTabID : int32_t {
    UNKNOWN = -1,
    OBJECTS,
    DETAILS,
    PROPERTIES
};

class EditorButton : public ButtonWidget {
public:
    explicit EditorButton(std::string name) : ButtonWidget(name) {}

    void apply_theme_defaults(const Theme& theme) override {
        ButtonWidget::apply_theme_defaults(theme);

        configure_all_styles([](Style& style) { style.font_size(24); });
    }
};

class EditorTab : public ButtonWidget {
public:
    explicit EditorTab(std::string name, EditorTabID id) : ButtonWidget(name) {
        m_id = id;
    }

    void apply_theme_defaults(const Theme& theme) override {
        ButtonWidget::apply_theme_defaults(theme);
        TransitionSpec transition(0.20f, easing::out_cubic);

        configure_all_styles([&transition](Style& style) {
            style.font_size(24);
            style.border(BorderEdge{0.0F, BorderType::None});
            style.margin_top(5.0f);
            style.background_color(rgba(0, 0, 0, 0), transition);
        });

        style(StyleType::HOVER).background_color(theme.background_secondary_color, transition);
        style(StyleType::ACTIVE).background_color(theme.background_secondary_color, transition);
    }

    EditorTabID id() const {
        return m_id;
    }

    void set_selected(bool value) {
        if (m_selected == value) {
            return;
        }

        m_selected = value;
        update_style();
    }

    virtual void build(Node* body) = 0;

protected:
    void add_title(Node& body, std::string title) {
        body.add<TextWidget>(std::move(title)).set_font(surface().get_primary_font(), 30.0F);
    }

private:
    void update_style() {
        auto theme = surface().theme();
        TransitionSpec transition(0.20f, easing::out_cubic);

        style(StyleType::DEFAULT).background_color(m_selected ? theme.accent_color : rgba(0, 0, 0, 0), transition);
        style(StyleType::HOVER)
            .background_color(m_selected ? theme.accent_hover_color : theme.background_secondary_color, transition);
        style(StyleType::ACTIVE).background_color(m_selected ? theme.accent_color : theme.background_secondary_color, transition);
    }

    EditorTabID m_id = EditorTabID::UNKNOWN;
    bool m_selected = false;
};

class ObjectsTab : public EditorTab {
public:
    explicit ObjectsTab() : EditorTab("objects", EditorTabID::OBJECTS) {}

    void build(Node* body) override {
        add_title(*body, "Objects");
        body->add<TextWidget>("TODO");
    }
};

class DetailsTab : public EditorTab {
public:
    explicit DetailsTab(Dash* game) : EditorTab("details", EditorTabID::DETAILS) {
        m_game = game;
    }

    void build(Node* body) override {
        add_title(*body, "Level details");
        auto* level = m_game->current_level();
        if (level == nullptr) {
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

        auto* name_field = &body->add<TextInputWidget>(m_name, "name", LabelPlacement::Above);
        name_field->set_size({grow(), fit()});

        auto* creator_field = &body->add<TextInputWidget>(m_creator, "creator", LabelPlacement::Above);
        creator_field->set_size({grow(), fit()});

        auto* description_field = &body->add<TextInputWidget>(m_description, "description", LabelPlacement::Above);
        description_field->set_size({grow(), fit()});

        auto* music_location_field = &body->add<FileDialogWidget>("music", LabelPlacement::Above);
        music_location_field->set_value(fs::path(m_music_location).filename().string());
        music_location_field->on_select([this, music_location_field](const FileDialogResult&) {
            m_music_location = music_location_field->value();
            music_location_field->set_value(fs::path(m_music_location).filename().string());
        });

        auto& update = body->add<EditorButton>("update");
        update.on_click([this, music_location_field] {
            update_metadata();
            music_location_field->set_value(fs::path(m_music_location).filename().string());
        });
    }

private:
    void update_metadata() {
        auto* level = m_game->current_level();
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

private:
    Dash* m_game;
    DashLevel* m_level = nullptr;

    std::string m_name;
    std::string m_creator;
    std::string m_description;
    std::string m_music_location;
};

class PropertiesTab : public EditorTab {
public:
    explicit PropertiesTab(Dash& game) : EditorTab("properties", EditorTabID::PROPERTIES), m_game(game) {}

    void build(Node* body) override {
        add_title(*body, "Properties");

        const DashLevel* level = m_game.current_level();
        Entity* selected = level != nullptr ? m_game.editor().selected_object(*level) : nullptr;
        if (selected == nullptr) {
            body->add<TextWidget>("select an object in the world");
            return;
        }

        build_object(*body, *selected);

        if (auto* rigid_body = selected->get_component<RigidBody>()) {
            build_rigid_body(*body, *rigid_body);
        }

        if (auto* sprite = selected->get_component<Sprite>()) {
            build_sprite(*body, *sprite);
        }
    }

private:
    static NumberInputWidget& add_number(Node& body, float& value, std::string label) {
        auto& field = body.add<NumberInputWidget>(value, std::move(label), LabelPlacement::Above);
        field.set_size({grow(), fit()});
        field.clear_range();
        return field;
    }

    void build_object(Node& body, Entity& selected) {
        body.add<TextWidget>(TextFormat("type: %s", selected.type_to_str()));
        m_z_index = selected.z_order();
        auto& z_index = body.add<NumberInputWidget>(m_z_index, "z index", LabelPlacement::Above);
        z_index.set_size({grow(), fit()});
        z_index.clear_range();
        z_index.on_change([this, object = &selected] { object->set_z_order(m_z_index); });

        m_visible = selected.is_visible();
        body.add<CheckboxWidget>(m_visible, "visible").on_change([this, object = &selected] { object->set_visible(m_visible); });
    }

    void build_rigid_body(Node& body, RigidBody& rigid_body) {
        body.add<TextWidget>("Rigid body").set_font(surface().get_primary_font(), 23.0F);

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

    void build_sprite(Node& body, Sprite& sprite) {
        body.add<TextWidget>("Sprite").set_font(surface().get_primary_font(), 23.0F);

        m_rotation = sprite.rotation;
        add_number(body, m_rotation, "rotation").on_change([this, &sprite] { sprite.rotation = m_rotation; });

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
            m_sprite_type, std::vector<DropdownOption>{{"square", "square"}, {"triangle", "triangle"}, {"circle", "circle"}},
            "shape"
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

    Dash& m_game;
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

class EditorTabs : public Container {
public:
    explicit EditorTabs(Node* body, Dash* game) : Container("editor-tabs", StackDirection::Horizontal), m_body(body) {
        set_size({grow(), fit()});
        set_content_alignment(Anchor::Center);

        configure_all_styles([](Style& style) {
            style.background_color(rgba(0, 0, 0, 0));
            style.border_top(1.0f);
        });

        // setup editor tabs
        add<ObjectsTab>();
        add<DetailsTab>(game);
        m_properties = &add<PropertiesTab>(*game);

        // handle click events to update body's content
        for (auto& node : children()) {
            auto tab = static_cast<EditorTab*>(node.get());
            tab->on_click([tab, this] { select_tab(*tab); });
        }
    }

    void show_properties() {
        if (m_current_tab == m_properties) {
            refresh_properties();
        } else {
            select_tab(*m_properties);
        }
    }

    void refresh_properties() {
        if (m_current_tab != m_properties) {
            return;
        }

        m_body->clear();
        m_properties->build(m_body);
    }

private:
    void select_tab(EditorTab& tab) {
        if (m_current_tab == &tab) {
            return;
        }

        if (m_current_tab != nullptr) {
            m_current_tab->set_selected(false);
        }

        m_body->clear();
        tab.set_selected(true);
        tab.build(m_body);
        m_current_tab = &tab;
    }

    EditorTab* m_current_tab = nullptr;
    PropertiesTab* m_properties = nullptr;
    Node* m_body = nullptr;
};

EditorPanel::EditorPanel(Dash& game, InputCallback on_key_press)
    : LayerContainer("editor-panel"), m_game(game), m_on_key_press(std::move(on_key_press)) {
    set_size({fit(), fit()});
    set_anchor(Anchor::BottomRight);

    set_input_mode(InputMode::Blocker);
    configure_all_styles([](Style& style) {
        style.background_color(rgba(16, 16, 16, 180));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(0.0f);
        style.padding({6.0f, 0.0f});
    });

    // create editor icon to update compact mode on click
    auto* edit_texture = game.surface().runtime().textures().add("edit", RESOURCES_LOCATION / "ui/edit.svg");
    m_edit_icon = &add<ImageWidget>(edit_texture);
    m_edit_icon->configure_all_styles([](Style& style) { style.color(rgb(255, 255, 255)); });
    m_edit_icon->style(StyleType::HOVER).cursor(ImGuiMouseCursor_Arrow);
    m_edit_icon->on_event([this](UiEvent& event) {
        if (event.type != EventType::Click || event.button != PointerButton::Left) {
            return;
        }

        set_compact(false);
        event.stop_propagation();
        event.block_native_input();
    });

    // create panel content
    m_content = &add<Container>("editor-panel-content");
    m_content->set_size({grow(), grow()});
    m_content->set_spacing(10.0F);

    // create panel header
    auto& header = m_content->add<Container>("editor-panel-header");
    header.set_size({grow(), fit()});
    header.set_content_alignment(Anchor::TopRight);

    // create close texture and update compact mode on click
    auto* close_texture = game.surface().runtime().textures().add("close", RESOURCES_LOCATION / "ui/close.svg");
    auto& close = header.add<ImageWidget>(close_texture);

    close.configure_all_styles([](Style& style) { style.color(rgb(255, 255, 255)); });
    close.style(StyleType::HOVER).background_color(rgba(255, 255, 255, 30)).border_radius(4.0F);
    close.on_event([this](UiEvent& event) {
        if (event.type != EventType::Click || event.button != PointerButton::Left) {
            return;
        }

        set_compact(true);

        event.stop_propagation();
        event.block_native_input();
    });

    // create tab's content
    auto content_body = &m_content->add<Container>("editor-panel-body");
    content_body->set_size({grow(), grow()});
    content_body->set_scrollable(true);
    content_body->set_spacing(8.0f);
    content_body->configure_all_styles([](Style& style) { style.padding({10.0f, 0.0f, 10.0f, 0.0f}); });

    m_tabs = &m_content->add<EditorTabs>(content_body, &game);

    // start compact
    set_compact(true);
}

void EditorPanel::on_update(float) {
    const std::size_t version = m_game.editor().selection_version();
    if (version == m_selection_version) {
        return;
    }

    m_selection_version = version;
    const DashLevel* level = m_game.current_level();
    if (level != nullptr && m_game.editor().selected_object(*level) != nullptr) {
        set_compact(false);
        m_tabs->show_properties();
    } else {
        m_tabs->refresh_properties();
    }
}

void EditorPanel::key_press_event(UiEvent& event) {
    if (m_on_key_press) m_on_key_press(event);
}

void EditorPanel::set_compact(bool compact) {
    if (compact) {
        set_size({fit(), fit()});
    } else {
        set_size({px(200), percent(100)}, {percent(30), percent(100)});
    }

    m_edit_icon->set_visible(compact);
    m_content->set_visible(!compact);

    const float padding = compact ? 0.0F : 10.0F;
    configure_all_styles([compact, padding](Style& style) {
        style.padding({padding, padding});
        style.background_color(rgba(20, 24, 32, compact ? 0 : 240));
        style.border(compact ? BorderEdge{0.0F, BorderType::None} : BorderEdge{1.0F, BorderType::Solid});
        style.border_color(rgba(140, 160, 190, 140));
        style.border_radius(compact ? 0.0F : 8.0F);
    });
}
