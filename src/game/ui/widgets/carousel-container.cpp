#include "carousel-container.hpp"

#include <algorithm>
#include <cmath>
#include <raylib.h>
#include <utility>

using namespace ui;

static bool is_carousel_item(const Node& node) {
    return node.visible() && !node.removal_pending() && node.layout().in_flow();
}

static bool is_mouse_within_window() {
    const Vector2 position = GetMousePosition();
    return position.x >= 0 && position.x <= static_cast<float>(GetScreenWidth()) && position.y >= 0 &&
           position.y <= static_cast<float>(GetScreenHeight());
}

CarouselContainer::CarouselContainer(std::string id) : Container(std::move(id), StackDirection::Horizontal, "CarouselContainer") {
    set_size({grow(), grow()});
    set_spacing(20.0F);
}

void CarouselContainer::apply_theme_defaults(const Theme& theme) {
    Container::apply_theme_defaults(theme);

    configure_all_styles([&theme](Style& style) {
        style.overflow(Overflow::Clip);
        style.background_color(theme.background_color);
    });
}

Container& CarouselContainer::set_spacing(float spacing) {
    m_spacing = std::max(0.0F, spacing);
    return Container::set_spacing(m_spacing);
}

CarouselContainer& CarouselContainer::select(std::size_t index) {
    m_selected = index;
    return *this;
}

void CarouselContainer::draw_children() {
    arrange_items();
    for (const auto& child : children()) {
        if (child->visible() && !child->removal_pending()) child->draw();
    }
}

void CarouselContainer::arrange_items() {
    const ImVec2 available = child_layout_size();

    float cursor = 0.0F;
    float target = 0.0F;
    std::size_t index = 0;

    for (const auto& child : children()) {
        if (!is_carousel_item(*child)) continue;

        const BoxInsets margin = child->layout_margin();
        const ImVec2 child_available = {
            std::max(0.0F, available.x - margin.horizontal()), std::max(0.0F, available.y - margin.vertical())
        };

        const ImVec2 size = child->layout().resolve_size(child_available);
        arrange_child(*child, size);

        if (index <= m_selected) {
            target = cursor + margin.left + size.x * 0.5F - available.x * 0.5F;
            m_stride = size.x + margin.horizontal() + m_spacing;
        }

        ++index;
        cursor += size.x + margin.horizontal() + m_spacing;
    }

    if (index == 0) {
        animator().cancel();
        m_press.reset();
        m_target.reset();
        m_selected = 0;
        return;
    }

    m_selected = std::min(m_selected, index - 1);
    if (!m_target) {
        m_offset = target;
    }

    if (!m_press && (target != m_target || (!animator().transitioning() && m_offset != target))) {
        animator().cancel();
        animator().animate().to(m_offset, target, {0.2F, easing::out_cubic});
    }

    m_target = target;
    cursor = -m_offset;

    for (const auto& child : children()) {
        if (!is_carousel_item(*child)) continue;

        const ImVec2 size = child->layout().size();
        const BoxInsets margin = child->layout_margin();
        Placement placement{};
        placement.offset = {cursor + margin.left, margin.top + (available.y - size.y - margin.vertical()) * 0.5F};
        arrange_child(*child, size, placement);
        cursor += size.x + margin.horizontal() + m_spacing;
    }
}

void CarouselContainer::event(UiEvent& input) {
    if (input.type != EventType::Cancel) return;

    m_press.reset();
    release_pointer();
}

void CarouselContainer::mouse_press_event(UiEvent& input) {
    if (input.button != PointerButton::Left || !capture_pointer()) return;

    // pause centering until release.
    animator().cancel();
    m_press = CarouselPress{input.position.x, m_offset};
}

void CarouselContainer::mouse_move_event(UiEvent& input) {
    if (!m_press || !is_mouse_within_window()) return;

    const float distance = input.position.x - m_press->x;
    if (!m_press->dragging && std::abs(distance) < 8.0F) return;

    m_press->dragging = true;
    animator().cancel();
    m_offset = m_press->offset - distance;
    input.stop_propagation();
}

void CarouselContainer::mouse_release_event(UiEvent& input) {
    if (input.button != PointerButton::Left || !m_press) return;

    const CarouselPress press = *m_press;
    m_press.reset();
    release_pointer();

    // select another card before allowing its click.
    if (!press.dragging) {
        std::size_t index = 0;
        for (const auto& child : children()) {
            if (!is_carousel_item(*child)) {
                continue;
            }

            if (child->layout().visual_rect().contains(input.position) && index != m_selected) {
                select(index);
                input.prevent_default();
                input.stop_propagation();
                return;
            }
            ++index;
        }
        return;
    }

    const float distance = input.position.x - press.x;
    if (std::abs(distance) >= m_stride * 0.25F) {
        if (distance < 0.0F) {
            select(m_selected + 1);
        } else if (m_selected > 0) {
            select(m_selected - 1);
        }
    }

    input.prevent_default();
    input.stop_propagation();
}
