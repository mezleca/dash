#include "dash-vector-input.hpp"

#include <imgui-ui/widgets/text.hpp>
#include <cassert>
#include <utility>

using namespace ui;

class VectorAxisInput : public DashNumberInput<float> {
public:
    VectorAxisInput(float& value, const char* label) : DashNumberInput<float>(value) {
        set_label(label, LabelPlacement::Inline);
        set_size({{px(88.0F), percent(100)}, fit()});
        clear_range();
    }

protected:
    void apply_theme_defaults(const Theme& theme) override {
        DashNumberInput<float>::apply_theme_defaults(theme);

        set_font(surface().get_primary_font(), 22.0F);
        set_spacing(4.0F);
        configure_all_styles([](Style& style) { style.padding({}); });
    }
};

DashVectorInput::DashVectorInput(Vector2& value, std::string label)
    : DashVectorInput(std::move(label), value.x, value.y, nullptr) {}

DashVectorInput::DashVectorInput(Vector3& value, std::string label)
    : DashVectorInput(std::move(label), value.x, value.y, &value.z) {}

DashVectorInput::DashVectorInput(std::string label, float& x, float& y, float* z) : Container({}, "DashVectorInput") {
    set_size({grow(), fit()});
    set_spacing(4.0F);
    set_input_mode(InputMode::None);

    add<TextWidget>(std::move(label)).set_input_mode(InputMode::None);

    auto& body = add<Container>("vector-body", StackDirection::Horizontal);

    body.set_size({grow(), fit()});
    body.set_wrap(StackWrap::Wrap);
    body.set_spacing(4.0F);
    body.set_input_mode(InputMode::None);

    const char* labels[] = {"x", "y", "z"};
    float* values[] = {&x, &y, z};

    for (std::size_t index = 0; index < m_fields.size(); ++index) {
        if (values[index] != nullptr) {
            m_fields[index] = &body.add<VectorAxisInput>(*values[index], labels[index]);
        }
    }
}

DashNumberInput<float>& DashVectorInput::component(VectorAxis axis) {
    const auto index = static_cast<std::size_t>(axis);

    assert(index < m_fields.size() && m_fields[index] != nullptr);

    return *m_fields[index];
}

void DashVectorInput::on_change(std::function<void()> callback) {
    for (auto* field : m_fields) {
        if (field != nullptr) {
            field->on_change(callback);
        }
    }
}

void DashVectorInput::set_minimum(float minimum) {
    for (auto* field : m_fields) {
        if (field != nullptr) {
            field->set_minimum(minimum);
        }
    }
}

void DashVectorInput::apply_theme_defaults(const Theme& theme) {
    Container::apply_theme_defaults(theme);

    set_font(surface().get_primary_font(), dash_ui::control_font_size);
    configure_all_styles([](Style& style) {
        style.padding({});
        style.border(BorderEdge{0.0F, BorderType::None});
        style.background_color(rgba(0, 0, 0, 0));
    });
}
