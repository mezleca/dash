#include "library-panel.hpp"

#include "game/dash.hpp"
#include "game/editor/library.hpp"
#include "game/ui/widgets/library-item.hpp"

using namespace ui;

LibraryPanel::LibraryPanel(Dash& game) : Container("editor-library", StackDirection::Horizontal) {
    set_wrap(StackWrap::Wrap);
    set_spacing(8.0F);
    set_scrollable(true);

    for (const auto& entry : EDITOR_LIBRARY) {
        add<LibraryItem>(entry).on_click([&game, &entry] {
            if (auto* editor = game.editor()) {
                editor->begin_add(entry);
            }
        });
    }
}
