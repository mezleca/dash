#pragma once

#include <imgui-ui/widgets/button.hpp>
#include <imgui-ui/widgets/checkbox.hpp>
#include <imgui-ui/widgets/dropdown.hpp>
#include <imgui-ui/file-dialog/file-dialog.hpp>

class DashButton : public ui::ButtonWidget {
public:
    explicit DashButton(std::string text, ui::LayoutSize size = {ui::fit(), ui::fit()});

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};

class DashDialog : public ui::Container {
public:
    explicit DashDialog(std::string label);

    const std::string& value() const;
    void set_value(std::string value);
    void on_select(std::function<void(const ui::FileDialogResult&)> callback);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void open_dialog();

    DashButton* m_button = nullptr;
    std::string m_value;
    std::function<void(const ui::FileDialogResult&)> m_on_select;
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
