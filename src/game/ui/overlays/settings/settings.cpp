#include "settings.hpp"
#include "game/dash.hpp"
#include "game/ui/controls.hpp"
#include "game/ui/widgets/dash-number-input.hpp"

#include <imgui-ui/widgets/text.hpp>

using namespace ui;

SettingsOverlay::SettingsOverlay(Dash& game, std::string id) : Overlay("settings-" + id), m_game(game) {
    m_volume = m_game.settings().volume();
    m_godmode = m_game.settings().godmode();
    m_free_mode = m_game.free_mode();

    set_size({percent(50.0F), percent(50.0F)}, {percent(80.0F), percent(80.0F)});
    set_content_alignment(Anchor::TopLeft);
    set_spacing(6.0F);
    configure_all_styles([](Style& style) { style.padding({16.0F, 12.0F}); });

    auto& heading = add<Container>("settings-heading");
    heading.set_size({grow(), fit()});
    heading.set_content_alignment(Anchor::Center);
    heading.add<TextWidget>("settings");

    auto& options = add<Container>("settings-options");
    options.set_size({grow(), fit()});
    options.set_spacing(4.0F);

    auto& volume = options.add<DashNumberInput<int>>(m_volume);
    volume.set_label("music volume", LabelPlacement::Above);
    volume.set_size({grow(), fit()});
    volume.set_range(0, 100);
    volume.on_change([this] { m_game.set_music_volume(m_volume); });

    auto& godmode = options.add<DashCheckbox>(m_godmode, "god mode");
    godmode.on_change([this] { m_game.set_godmode(m_godmode); });

    auto& free_mode = options.add<DashCheckbox>(m_free_mode, "free mode");
    free_mode.on_change([this] { m_game.set_free_mode(m_free_mode); });
}
