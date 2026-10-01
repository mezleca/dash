#include "settings.hpp"
#include "../../game.hpp"

#include <imgui-ui/widgets/checkbox.hpp>
#include <imgui-ui/widgets/box.hpp>
#include <imgui-ui/widgets/number-input.hpp>
#include <imgui-ui/widgets/text.hpp>

using namespace ui;

SettingsLayer::SettingsLayer(std::string id) : MenuOptionLayer("settings-" + id) {
    m_volume = game.settings().volume();
    m_godmode = game.settings().godmode();
    m_free_mode = game.free_mode();

    set_content_alignment(Anchor::TopLeft);
    set_spacing(10.0F);
    configure_all_styles([](Style& style) { style.padding({32.0F, 24.0F}); });

    add<TextWidget>("settings");

    auto& separator = add<BoxWidget>("settings-separator", LayoutSize{grow(), px(1)});
    separator.configure_all_styles([](Style& style) { style.background_color(rgb(72, 72, 72)); });

    auto& options = add<Container>("settings-options");
    options.set_size({grow(), fit()});
    options.set_spacing(12.0F);

    auto& volume = options.add<NumberInputWidget>(m_volume);
    volume.set_size({px(300), fit()});
    volume.set_label("music volume");
    volume.set_range(0, 100);
    volume.on_change([this] { game.set_music_volume(m_volume); });

    auto& godmode = options.add<CheckboxWidget>(m_godmode, "god mode");
    godmode.on_change([this] { game.set_godmode(m_godmode); });

    auto& free_mode = options.add<CheckboxWidget>(m_free_mode, "free mode");
    free_mode.on_change([this] { game.set_free_mode(m_free_mode); });
}

ImGuiWindowFlags SettingsLayer::child_window_flags() const {
    return Container::child_window_flags();
}
