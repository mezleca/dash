#pragma once

#include <imgui-ui/widgets/image.hpp>

#include <functional>
#include <utility>

class DashIconButton : public ui::ImageWidget {
public:
    explicit DashIconButton(ui::Texture* icon);

    DashIconButton& on_click(std::function<void()> callback) {
        m_on_click = std::move(callback);
        return *this;
    }

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
    void event(ui::UiEvent& event) override;

private:
    std::function<void()> m_on_click;
};
