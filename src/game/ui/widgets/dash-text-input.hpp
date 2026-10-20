#pragma once

#include <imgui-ui/widgets/text-input.hpp>

class DashTextInput : public ui::TextInputWidget {
public:
    explicit DashTextInput(std::string& value, std::string id = {});

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;
};
