#pragma once

#include "dash-number-input.hpp"

#include <raylib.h>
#include <array>
#include <functional>
#include <string>

enum class VectorAxis {
    X,
    Y,
    Z,
};

class DashVectorInput : public ui::Container {
public:
    DashVectorInput(Vector2& value, std::string label);
    DashVectorInput(Vector3& value, std::string label);

    DashNumberInput<float>& component(VectorAxis axis);
    void on_change(std::function<void()> callback);
    void set_minimum(float minimum);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    DashVectorInput(std::string label, float& x, float& y, float* z);
    std::array<DashNumberInput<float>*, 3> m_fields = {};
};
