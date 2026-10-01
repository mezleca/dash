#include "exit.hpp"

#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/surface.hpp>
#include <imgui-ui/widgets/box.hpp>
#include <imgui-ui/widgets/button.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

ExitLayer::ExitLayer(std::string id, std::function<void()> on_cancel) : MenuOptionLayer("exit-" + id) {
    set_content_alignment(Anchor::TopLeft);
    set_spacing(10.0F);
    configure_all_styles([](Style& style) { style.padding({32.0F, 24.0F}); });

    add<TextWidget>("exit game?");

    auto& separator = add<BoxWidget>("exit-separator", LayoutSize{grow(), px(1)});
    separator.configure_all_styles([](Style& style) { style.background_color(rgb(72, 72, 72)); });

    auto& actions = add<Container>("exit-actions", StackDirection::Horizontal);
    actions.set_size({grow(), fit()});
    actions.set_spacing(8.0F);

    actions.add<ButtonWidget>("confirm").on_click([this] { surface().exit(); });
    actions.add<ButtonWidget>("cancel").on_click(std::move(on_cancel));
}
