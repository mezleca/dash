#include "exit.hpp"

#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/button.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

ExitLayer::ExitLayer(std::string id, std::function<void()> on_cancel) : MenuOptionLayer("exit-" + id) {
    auto& modal = add<Container>("exit-modal");
    modal.set_size({fit(), fit()});
    modal.set_spacing(16.0F);

    modal.add<TextWidget>("exit game?");

    auto& actions = modal.add<Container>("exit-actions", StackDirection::Horizontal);
    actions.set_size({fit(), fit()});
    actions.set_spacing(12.0F);

    actions.add<ButtonWidget>("confirm").on_click([this] { surface().exit(); });
    actions.add<ButtonWidget>("cancel").on_click(std::move(on_cancel));
}
