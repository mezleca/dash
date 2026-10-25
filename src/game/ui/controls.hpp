#pragma once

#include <imgui-ui/widgets/button.hpp>
#include <imgui-ui/widgets/image.hpp>
#include <imgui-ui/widgets/checkbox.hpp>
#include <imgui-ui/widgets/dropdown.hpp>
#include <imgui-ui/widgets/text-input.hpp>
#include <imgui-ui/widgets/color-picker.hpp>
#include <imgui-ui/file-dialog/file-dialog.hpp>
#include <functional>
#include <string>
#include <vector>

class DashIconButton : public ui::ImageWidget {
public:
    explicit DashIconButton(ui::Texture* icon = nullptr);
    DashIconButton& on_click(std::function<void()> callback);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void event(ui::UiEvent& event) override;

private:
    std::function<void()> m_on_click;
};

class DashCloseButton : public DashIconButton {
protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashButton : public ui::ButtonWidget {
public:
    explicit DashButton(std::string text, ui::LayoutSize size = {ui::fit(), ui::fit()});

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashTargetButton : public DashButton {
public:
    using DashButton::DashButton;

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashDeleteButton : public DashButton {
public:
    explicit DashDeleteButton(std::string text = "delete");

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashDialog : public ui::Container {
public:
    explicit DashDialog(std::string label);

    const std::string& value() const;
    void set_value(std::string value);
    void on_select(std::function<void(const ui::FileDialogResult&)> callback);
    void set_filters(std::vector<ui::FileDialogFilter> filters);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void open_dialog();

    DashButton* m_button = nullptr;
    std::string m_value;
    std::function<void(const ui::FileDialogResult&)> m_on_select;
    ui::FileDialogOptions m_options;
};

class DashCheckbox : public ui::CheckboxWidget {
public:
    using ui::CheckboxWidget::CheckboxWidget;

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashDropdown : public ui::DropdownWidget {
public:
    using ui::DropdownWidget::DropdownWidget;

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashTextInput : public ui::TextInputWidget {
public:
    using ui::TextInputWidget::TextInputWidget;

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashColorPicker : public ui::ColorPickerWidget {
public:
    explicit DashColorPicker(
        ui::WidgetValue<ui::Color> value = {}, std::string label = {}, ui::LabelPlacement placement = ui::LabelPlacement::Inline,
        std::string id = {}
    );

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
