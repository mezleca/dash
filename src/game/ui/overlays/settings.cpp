#include "settings.hpp"

#include <imgui-ui/widgets/text.hpp>

using namespace ui;

SettingsLayer::SettingsLayer(std::string id) : MenuOptionLayer("settings-" + id) {
    set_content_alignment(Anchor::Center);
    add<TextWidget>("settings");
}
