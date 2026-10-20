#pragma once

#include <imgui-ui/widgets/image.hpp>

#include <functional>
#include <utility>

class EditorModeButton : public ui::ImageWidget {
public:
    EditorModeButton(ui::Texture* icon, bool selected);

    EditorModeButton& on_click(std::function<void()> callback) {
        m_on_click = std::move(callback);
        return *this;
    }

    void set_selected(bool value);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void event(ui::UiEvent& event) override;

private:
    void update_style(const ui::Theme& theme);

    std::function<void()> m_on_click;
    bool m_selected = false;
};
