#include "controls.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

DashButton::DashButton(std::string text, LayoutSize size) : ButtonWidget(std::move(text), size) {
    set_text_overflow(TextOverflow::Ellipsis);
}

void DashButton::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);

    configure_all_styles([](Style& style) {
        style.font_size(24);
        style.overflow(Overflow::Hidden);
    });
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
    set_font(surface().get_primary_font(), 26.0F);
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

void DashDialog::open_dialog() {
    const FileDialogResult result = surface().file_dialog().show(FileDialogOperation::OpenFile, {});
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
    set_font(surface().get_primary_font(), 26.0F);
}

void DashDropdown::apply_theme_defaults(const Theme& theme) {
    DropdownWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 26.0F);
}
