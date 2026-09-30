#pragma once

#include <imgui-ui/layout/container.hpp>

#include <cstddef>
#include <optional>

class CarouselContainer : public ui::Container {
public:
    explicit CarouselContainer(std::string id);

    CarouselContainer& select(std::size_t index);
    ui::Container& set_spacing(float spacing) override;
    std::size_t selected_index() const {
        return m_selected;
    }

protected:
    void arrange_children() override;
    void event(ui::UiEvent& event) override;
    void mouse_press_event(ui::UiEvent& event) override;
    void mouse_move_event(ui::UiEvent& event) override;
    void mouse_release_event(ui::UiEvent& event) override;

private:
    struct Press {
        float x;
        float offset;
        bool dragging = false;
    };

    std::optional<Press> m_press;
    std::size_t m_selected = 0;
    float m_offset = 0.0F;
    std::optional<float> m_target;
    float m_stride = 0.0F;
    float m_spacing = 20.0F;
};
