#include "exit.hpp"
#include "game/ui/controls.hpp"

#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

ExitOverlay::ExitOverlay(std::string id, std::string message, std::function<void()> on_confirm, std::function<void()> on_cancel)
    : Overlay("exit-" + id) {
    set_content_alignment(Anchor::Center);
    set_spacing(10.0F);
    configure_all_styles([](Style& style) { style.padding({32.0F, 24.0F}); });

    add<TextWidget>(std::move(message));

    auto& actions = add<Container>("exit-actions", StackDirection::Horizontal);
    actions.set_size({grow(), fit()});
    actions.set_content_alignment(Anchor::Center);
    actions.set_spacing(8.0F);

    actions.add<DashButton>("confirm").on_click(std::move(on_confirm));
    actions.add<DashButton>("cancel").on_click(std::move(on_cancel));
}
