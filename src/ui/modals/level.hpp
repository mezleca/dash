#pragma once

#include "../modal.hpp"

#include <cstdint>

struct UI;

struct LevelSelectorModal : public UIModal {
    explicit LevelSelectorModal(UI* ui);

    void on_remove() override;
    void on_escape() override;
    void render() override;

  private:
    int m_focused_item = 0;
    float m_holding_offset_x = 0.0f;
    float m_holding_start_x = 0.0f;
    float m_target_pos_x = 0.0f;
    float m_current_pos_x = 0.0f;
    int64_t m_last_snap_ms = 0;
    bool m_mouse_held = false;
    bool m_is_dragging = false;
};
