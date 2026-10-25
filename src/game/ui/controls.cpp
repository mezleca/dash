#include "controls.hpp"
#include "theme.hpp"
#include "game/paths.hpp"
#include <imgui-ui/runtime.hpp>

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

DashIconButton::DashIconButton(ui::Texture* icon) : ImageWidget(icon) {
    set_input_mode(InputMode::Target);
    set_size({px(30.0F), px(30.0F)});
    set_fit(ImageFit::Contain);
}

DashIconButton& DashIconButton::on_click(std::function<void()> callback) {
    m_on_click = std::move(callback);

    return *this;
}

void DashIconButton::apply_theme_defaults(const Theme& theme) {
    ImageWidget::apply_theme_defaults(theme);
    const auto& transition = dash_ui::control_transition;

    configure_all_styles([&](Style& style) {
        style.padding({5.0F, 5.0F});
        style.border(BorderEdge{0.0F, BorderType::None});
        style.border_radius(theme.controls.rounding);
        style.background_color(rgba(0, 0, 0, 0), transition);
        style.color(theme.text_secondary_color, transition);
    });

    style(StyleType::HOVER).background_color(rgba(149, 204, 221, 40), transition).color(theme.text_color, transition);
    style(StyleType::ACTIVE).background_color(rgba(66, 116, 217, 120), transition);
}

void DashIconButton::event(UiEvent& input) {
    if (input.button != PointerButton::Left) {
        return;
    }

    if (input.type == EventType::Click && m_on_click) {
        m_on_click();
    }

    if (input.type == EventType::Click || input.type == EventType::PointerDown) {
        input.stop_propagation();
    }
}

void DashCloseButton::apply_theme_defaults(const Theme& theme) {
    DashIconButton::apply_theme_defaults(theme);
    set_texture(surface().runtime().textures().add("dash-close", RESOURCES_LOCATION / "ui/close.svg"));
}

DashButton::DashButton(std::string text, LayoutSize size) : ButtonWidget(std::move(text), size) {
    set_text_overflow(TextOverflow::Ellipsis);
}

void DashButton::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);
    const auto& transition = dash_ui::control_transition;

    configure_all_styles([&](Style& style) {
        style.font_size(dash_ui::control_font_size);
        style.overflow(Overflow::Hidden);
        style.background_color(style.background_color().value, transition);
        style.border_color(style.border_color().value, transition);
        style.color(style.color().value, transition);
    });
}

void DashTargetButton::apply_theme_defaults(const Theme& theme) {
    DashButton::apply_theme_defaults(theme);
    const auto& transition = dash_ui::control_transition;

    configure_all_styles([&](Style& style) { style.border_color(theme.accent_color, transition); });
    style(StyleType::DEFAULT).background_color(theme.accent_color, transition);
    style(StyleType::HOVER).background_color(theme.accent_hover_color, transition);
    style(StyleType::ACTIVE).background_color(theme.controls.active_color, transition);
}

DashDeleteButton::DashDeleteButton(std::string text) : DashButton(std::move(text)) {}

void DashDeleteButton::apply_theme_defaults(const Theme& theme) {
    DashButton::apply_theme_defaults(theme);
    const auto& transition = dash_ui::control_transition;

    configure_all_styles([&](Style& style) { style.color(rgb(255, 255, 255), transition); });
    style(StyleType::DEFAULT).background_color(rgb(150, 35, 45), transition);
    style(StyleType::HOVER).background_color(rgb(185, 45, 55), transition);
    style(StyleType::ACTIVE).background_color(rgb(120, 25, 35), transition);
}

DashDialog::DashDialog(std::string label) : Container("dash-dialog") {
    set_size({grow(), fit()});
    set_spacing(10.0F);

    auto& heading = add<TextWidget>(std::move(label));

    heading.set_size({grow(), fit()});

    m_button = &add<DashButton>("select a file");
    m_button->set_size({grow(), fit()});
    m_button->set_text_alignment({0.0F, 0.5F});
    m_button->on_click([this] { open_dialog(); });
}

void DashDialog::apply_theme_defaults(const Theme& theme) {
    Container::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), dash_ui::control_font_size);

    configure_all_styles([](Style& style) {
        style.padding({});
        style.border(BorderEdge{0.0F, BorderType::None});
    });
}

const std::string& DashDialog::value() const {
    return m_value;
}

void DashDialog::set_value(std::string value) {
    m_value = std::move(value);
    m_button->set_text(m_value.empty() ? "select a file" : m_value);
}

void DashDialog::on_select(std::function<void(const FileDialogResult&)> callback) {
    m_on_select = std::move(callback);
}

void DashDialog::set_filters(std::vector<FileDialogFilter> filters) {
    m_options.filters = std::move(filters);
}

void DashDialog::open_dialog() {
    const FileDialogResult result = surface().file_dialog().show(FileDialogOperation::OpenFile, m_options);
    if (!result.accepted()) {
        return;
    }

    if (!result.paths.empty()) {
        set_value(result.paths.front().string());
    }

    if (m_on_select) {
        m_on_select(result);
    }
}

void DashCheckbox::apply_theme_defaults(const Theme& theme) {
    CheckboxWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), dash_ui::control_font_size);
}

void DashDropdown::apply_theme_defaults(const Theme& theme) {
    DropdownWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), dash_ui::control_font_size);
}

void DashTextInput::apply_theme_defaults(const Theme& theme) {
    TextInputWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), dash_ui::control_font_size);

    if (auto* input = dynamic_cast<StyledNode*>(find("input"))) {
        dash_ui::input_style(*input, theme);
    }
}

DashColorPicker::DashColorPicker(WidgetValue<ui::Color> value, std::string label, LabelPlacement placement, std::string id)
    : ColorPickerWidget(std::move(value), std::move(label), placement, std::move(id)) {
    set_content_alignment({0.0F, 0.5F});
}

void DashColorPicker::apply_theme_defaults(const Theme& theme) {
    ColorPickerWidget::apply_theme_defaults(theme);

    set_font(surface().get_primary_font(), dash_ui::control_font_size);
    label().set_font(surface().get_primary_font(), dash_ui::control_font_size);
    preview().set_size({px(48.0F), px(28.0F)});
}
