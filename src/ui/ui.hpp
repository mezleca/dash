#pragma once

enum class UIState : int { NONE = 0, MENU = 1 << 0, OPTIONS = 1 << 1, PLAYFIELD = 1 << 2 };

inline UIState operator|(UIState a, UIState b) {
    return static_cast<UIState>(static_cast<int>(a) | static_cast<int>(b));
}

inline UIState operator&(UIState a, UIState b) {
    return static_cast<UIState>(static_cast<int>(a) & static_cast<int>(b));
}

inline UIState operator~(UIState a) {
    return static_cast<UIState>(~static_cast<int>(a));
}

inline UIState& operator|=(UIState& a, UIState b) {
    return a = a | b;
}
inline UIState& operator&=(UIState& a, UIState b) {
    return a = a & b;
}

struct UI {
    UIState state;

    bool is_on_playfield() {
        return (state & UIState::PLAYFIELD) != UIState::NONE;
    }
} inline g_ui;
