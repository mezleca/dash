#pragma once

#include <imgui-ui/widgets/image.hpp>
#include "game/ui/controls.hpp"
#include <imgui-ui/widgets/text-input.hpp>

#include <functional>
#include <utility>

class EditorIconButton : public ui::ImageWidget {
public:
    explicit EditorIconButton(ui::Texture* icon);

    EditorIconButton& on_click(std::function<void()> callback) {
        m_on_click = std::move(callback);
        return *this;
    }

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void event(ui::UiEvent& event) override;

private:
    std::function<void()> m_on_click;
};

class EditorModeButton : public EditorIconButton {
public:
    EditorModeButton(ui::Texture* icon, bool selected);
    void set_selected(bool value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void update_style(const ui::Theme& theme);

    bool m_selected = false;
};

class EditorTabButton : public DashButton {
public:
    explicit EditorTabButton(std::string name);
    void set_selected(bool value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    void update_style();

    bool m_selected = false;
};

class EditorTextInput : public ui::TextInputWidget {
public:
    explicit EditorTextInput(std::string& value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
