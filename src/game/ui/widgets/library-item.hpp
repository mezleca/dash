#pragma once

#include "game/ui/controls.hpp"

struct LibraryEntry;

namespace ui {
    class ImageWidget;
    class TextWidget;
} // namespace ui

class LibraryItem : public DashButton {
public:
    explicit LibraryItem(const LibraryEntry& entry);

protected:
    void apply_theme_defaults(const ui::Theme& theme) override;

private:
    const LibraryEntry& m_entry;
    ui::ImageWidget* m_icon = nullptr;
    ui::TextWidget* m_label = nullptr;
};
