#include "carousel-container.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

using namespace ui;

static bool is_carousel_item(const Node& node) {
    return node.visible() && !node.removal_pending() && node.layout().in_flow();
}

CarouselContainer::CarouselContainer(std::string id) : Container(std::move(id), StackDirection::Horizontal, "CarouselContainer") {
    set_size({grow(), grow()});
    set_spacing(20.0F);
    configure_all_styles([](Style& style) { style.overflow(Overflow::Clip); });
}

Container& CarouselContainer::set_spacing(float spacing) {
    m_spacing = std::max(0.0F, spacing);
    return Container::set_spacing(m_spacing);
}

CarouselContainer& CarouselContainer::select(std::size_t index) {
    m_selected = index;
    return *this;
}

void CarouselContainer::arrange_children() {
    // measure the row and find the offset that centers the selected item.
    const ImVec2 available = child_layout_size();
    float cursor = 0.0F;
    float target = 0.0F;
    std::size_t index = 0;
    for (const auto& child : children()) {
        if (!is_carousel_item(*child)) continue;

        const ImVec2 margin = child->layout_margin();
        const ImVec2 size = child->layout().resolve_size(
            {std::max(0.0F, available.x - margin.x * 2.0F), std::max(0.0F, available.y - margin.y * 2.0F)}
        );
        arrange_child(*child, size);
        if (index <= m_selected) {
            target = cursor + margin.x + size.x * 0.5F - available.x * 0.5F;
            m_stride = size.x + margin.x * 2.0F + m_spacing;
        }
        ++index;
        cursor += size.x + margin.x * 2.0F + m_spacing;
    }

    if (index == 0) {
        animator().cancel();
        m_press.reset();
        m_target.reset();
        m_selected = 0;
        return;
    }
    m_selected = std::min(m_selected, index - 1);

    if (!m_target) m_offset = target;

    if (!m_press && (target != m_target || (!animator().transitioning() && m_offset != target))) {
        animator().cancel();
        animator().animate().to(m_offset, target, {0.2F, easing::out_cubic});
    }
    m_target = target;

    cursor = -m_offset;
    for (const auto& child : children()) {
        if (!is_carousel_item(*child)) continue;

        const ImVec2 size = child->layout().size();
        const ImVec2 margin = child->layout_margin();
        Placement placement{};
        placement.offset = {cursor + margin.x, (available.y - size.y) * 0.5F};
        arrange_child(*child, size, placement);
        cursor += size.x + margin.x * 2.0F + m_spacing;
    }
}

void CarouselContainer::event(UiEvent& input) {
    if (input.type != EventType::Cancel) return;

    m_press.reset();
    release_pointer();
}

void CarouselContainer::mouse_press_event(UiEvent& input) {
    if (input.button != PointerButton::Left || !capture_pointer()) return;

    // stop centering while pressed so the card stays under the pointer until release.
    animator().cancel();
    m_press = Press{input.position.x, m_offset};
}

void CarouselContainer::mouse_move_event(UiEvent& input) {
    if (!m_press) return;

    const float distance = input.position.x - m_press->x;
    if (!m_press->dragging && std::abs(distance) < 8.0F) return;

    m_press->dragging = true;
    animator().cancel();
    m_offset = m_press->offset - distance;
    input.stop_propagation();
}

void CarouselContainer::mouse_release_event(UiEvent& input) {
    if (input.button != PointerButton::Left || !m_press) return;

    const Press press = *m_press;
    m_press.reset();
    release_pointer();

    // suppress the card's click when release selects a different item.
    if (!press.dragging) {
        std::size_t index = 0;
        for (const auto& child : children()) {
            if (!is_carousel_item(*child)) continue;

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
        if (distance < 0.0F)
            select(m_selected + 1);
        else if (m_selected > 0)
            select(m_selected - 1);
    }
    input.prevent_default();
    input.stop_propagation();
}
