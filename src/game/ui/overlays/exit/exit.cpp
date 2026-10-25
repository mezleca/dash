#include "exit.hpp"
#include "game/ui/controls.hpp"

#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

ExitOverlay::ExitOverlay(std::string id, std::string message, std::function<void()> on_confirm, std::function<void()> on_cancel)
    : Overlay("exit-" + id) {
    content().set_content_alignment(Anchor::Center);
    content().set_spacing(10.0F);

    content().add<TextWidget>(std::move(message));

    auto& actions = content().add<Container>("exit-actions", StackDirection::Horizontal);
    actions.set_size({grow(), fit()});
    actions.set_content_alignment(Anchor::Center);
    actions.set_spacing(8.0F);

    actions.add<DashButton>("confirm").on_click(std::move(on_confirm));
    actions.add<DashButton>("cancel").on_click(std::move(on_cancel));
}
