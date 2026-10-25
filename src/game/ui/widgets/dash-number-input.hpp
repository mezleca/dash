#pragma once

#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/number-input.hpp>
#include "game/ui/theme.hpp"

#include <string>
#include <utility>

template <ui::NumberInputType T>
class DashNumberInput : public ui::NumberInputWidget<T> {
public:
    explicit DashNumberInput(T& value, std::string id = {})
        : ui::NumberInputWidget<T>(value, std::move(id), ui::LabelPlacement::Above) {}

protected:
    void apply_theme_defaults(const ui::Theme& theme) override {
        ui::NumberInputWidget<T>::apply_theme_defaults(theme);
        this->set_font(this->surface().get_primary_font(), dash_ui::control_font_size);
        this->set_thumb_visible(false);

        const auto& transition = dash_ui::control_transition;

        this->configure_all_styles([&theme, &transition](ui::Style& style) {
            style.padding({6.0F, 3.0F});
            style.border(ui::BorderEdge{0.0F, ui::BorderType::None});
            style.border_radius(theme.controls.rounding);
            style.background_color(ui::rgba(0, 0, 0, 0), transition);
        });

        this->style(ui::StyleType::ACTIVE).border_color(theme.accent_color, transition);

        if (auto* field = dynamic_cast<ui::StyledNode*>(this->find("input"))) {
            dash_ui::input_style(*field, theme);
        }
    }
};
