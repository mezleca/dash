#include "library-item.hpp"

#include "game/editor/library.hpp"
#include "game/paths.hpp"

#include <imgui-ui/runtime.hpp>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/image.hpp>
#include <imgui-ui/widgets/text.hpp>

using namespace ui;

static fs::path library_icon_path(const char* icon) {
    return RESOURCES_LOCATION / "ui/editor/objects" / icon;
}

LibraryItem::LibraryItem(const LibraryEntry& entry) : DashButton("", {px(96.0F), px(80.0F)}), m_entry(entry) {
    set_direction(StackDirection::Vertical);
    set_content_alignment(Anchor::Center);
    set_spacing(4.0F);

    m_icon = &add<ImageWidget>();
    m_icon->set_size({px(32.0F), px(32.0F)});
    m_icon->set_fit(ImageFit::Contain);
    m_icon->set_input_mode(InputMode::None);

    m_label = &add<TextWidget>(entry.m_name);
    m_label->set_input_mode(InputMode::None);
}

void LibraryItem::apply_theme_defaults(const Theme& theme) {
    DashButton::apply_theme_defaults(theme);
    m_label->set_font(surface().get_primary_font(), 18.0F);

    m_icon->set_texture(
        surface().runtime().textures().add(std::string("editor-object-") + m_entry.m_icon, library_icon_path(m_entry.m_icon))
    );
}
