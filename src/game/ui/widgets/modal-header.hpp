#pragma once

#include <imgui-ui/layout/container.hpp>
#include <functional>
#include <string>

namespace ui {
    class TextWidget;
}

class DashModalHeader : public ui::Container {
public:
    explicit DashModalHeader(std::string title = {}, std::function<void()> on_close = {});
    void set_title(std::string title);

private:
    ui::Container* m_title_slot = nullptr;
    ui::TextWidget* m_title = nullptr;
};
