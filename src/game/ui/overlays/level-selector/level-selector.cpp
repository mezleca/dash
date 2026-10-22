#include "level-selector.hpp"
#include "../../widgets/level-card.hpp"
#include "../../widgets/carousel-container.hpp"
#include "game/dash.hpp"
#include "game/ui/controls.hpp"
#include "game/ui/overlays/editor/controls.hpp"

#include <imgui-ui/widgets/text.hpp>
#include <imgui-ui/surface.hpp>
#include <utility>

using namespace ui;

LevelSelectorOverlay::LevelSelectorOverlay(Dash& game, std::string id, std::function<void(DashLevel&)> on_select)
    : Overlay("level-selector-" + id), m_game(game), m_on_select(std::move(on_select)) {
    set_size({grow(), grow()});

    m_carousel = &add<CarouselContainer>("levels");
    m_create_card = &m_carousel->add<DashButton>("create", LayoutSize{percent(50), percent(50)});
    m_create_card->set_visible(false);
    m_create_card->configure_all_styles([](Style& style) {
        style.font_size(48);
        style.background_color(rgb(36, 36, 36));
        style.padding({16.0F, 16.0F});
        style.border_radius(4.0F);
        style.border_thickness(2.0F);
    });
    m_create_card->on_click([this] { show_create_panel(); });

    for (auto& entry : game.levels()) {
        add_level(*entry.second);
    }

    m_empty_label = &add<TextWidget>("no levels found");
    m_empty_label->set_anchor(Anchor::Center);
    m_empty_label->set_visible(game.levels().empty());

    m_create_panel = &add<LayerContainer>("create-level", LayerMode::Window);
    m_create_panel->set_size({grow(), grow()});
    m_create_panel->set_input_mode(InputMode::Blocker);
    m_create_panel->set_visible(false);
    m_create_panel->set_content_alignment(Anchor::Center);
    m_create_panel->configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 180));
        style.border(BorderEdge{0.0F, BorderType::None});
    });

    auto& form = m_create_panel->add<Container>("create-level-form");
    form.set_size({{px(420), px(0), percent(100)}, {fit(), px(0), percent(100)}});
    form.set_scrollable(true);
    form.set_spacing(10.0F);
    form.configure_all_styles([](Style& style) {
        style.background_color(rgb(33, 34, 38));
        style.padding({24.0F, 20.0F});
        style.border_radius(4.0F);
    });

    auto& label = form.add<TextWidget>("create level");
    label.configure_all_styles([](Style& style) { style.font_size(26.0f); });

    auto& name = form.add<EditorTextInput>(m_name);
    name.set_label("name", LabelPlacement::Above);
    name.set_size({grow(), fit()});

    auto& creator = form.add<EditorTextInput>(m_creator);
    creator.set_label("creator", LabelPlacement::Above);
    creator.set_size({grow(), fit()});

    auto& description = form.add<EditorTextInput>(m_description);
    description.set_label("description", LabelPlacement::Above);
    description.set_size({grow(), fit()});

    m_music_field = &form.add<DashDialog>("music");
    m_error_label = &form.add<TextWidget>("could not create level, check the name, creator and music file");
    m_error_label->set_wrap(340.0F);
    m_error_label->set_visible(false);

    auto& actions = form.add<Container>("create-level-actions", StackDirection::Horizontal);
    actions.set_size({grow(), fit()});
    actions.set_content_alignment(Anchor::CenterRight);
    actions.set_spacing(8.0F);
    actions.add<DashButton>("cancel").on_click([this] { cancel_creation(); });
    m_confirm_button = &actions.add<DashButton>("create");
    m_confirm_button->set_enabled(false);
    m_confirm_button->on_click([this] { create_level(); });
}

void LevelSelectorOverlay::add_level(DashLevel& level) {
    auto& card = m_carousel->add<LevelCard>(level);

    card.set_removable(m_editor_mode);
    card.on_remove([this, &level, &card] {
        if (!m_game.remove_level(level)) return;
        m_carousel->remove(card);
        m_empty_label->set_visible(m_game.levels().empty() && !m_editor_mode);
    });

    if (m_on_select) {
        card.on_click([this, selected = &level] { m_on_select(*selected); });
    }
}

void LevelSelectorOverlay::set_editor_mode(bool editor) {
    m_editor_mode = editor;
    m_create_card->set_visible(editor);
    m_empty_label->set_visible(m_game.levels().empty() && !editor);
    for (const auto& child : m_carousel->children()) {
        if (child.get() == m_create_card) continue;
        static_cast<LevelCard&>(*child).set_removable(editor);
    }
    m_carousel->select(0);
    cancel_creation();
}

void LevelSelectorOverlay::show_create_panel() {
    m_name.clear();
    m_creator.clear();
    m_description.clear();
    m_music_field->set_value({});
    m_error_label->set_visible(false);
    m_confirm_button->set_enabled(false);

    parent()->add(m_create_panel->parent()->detach(*m_create_panel));
    m_create_panel->set_visible(true);
    surface().input_router().set_focus(m_create_panel);
    ImGui::SetWindowFocus(m_create_panel->id().c_str());
}

void LevelSelectorOverlay::on_update(float) {
    m_confirm_button->set_enabled(m_name.find_first_not_of(" \t\r\n") != std::string::npos && !m_music_field->value().empty());
}

bool LevelSelectorOverlay::cancel_creation() {
    if (!m_create_panel->visible()) return false;
    m_create_panel->set_visible(false);
    return true;
}

void LevelSelectorOverlay::create_level() {
    if (!m_confirm_button->enabled()) return;

    LevelMetadata metadata;
    metadata.name = m_name;
    metadata.creator = m_creator;
    metadata.description = m_description;
    metadata.music = DashFile(m_music_field->value());

    auto* level = m_game.create_level(metadata);
    if (level == nullptr) {
        m_error_label->set_visible(true);
        return;
    }

    add_level(*level);
    m_empty_label->set_visible(false);
    m_carousel->select(m_game.levels().size());
    cancel_creation();
    if (m_on_select) m_on_select(*level);
}

void LevelSelectorOverlay::apply_theme_defaults(const Theme& theme) {
    Overlay::apply_theme_defaults(theme);

    configure_all_styles([&theme](Style& style) {
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({});
        style.background_color(theme.background_color);
    });
}
