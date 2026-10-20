#include "level-card.hpp"
#include "game/level/level.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/imgui/painter.hpp>
#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/widgets/text.hpp>

#include <string>

using namespace ui;

class LevelProgressBar final : public Widget {
public:
    explicit LevelProgressBar(const DashLevel& level)
        : Widget("level-progress", "LevelProgress", InputMode::None), m_level(level) {
        set_size({percent(80), px(30.0F)});
    }

protected:
    void apply_theme_defaults(const Theme& theme) override {
        Widget::apply_theme_defaults(theme);
        set_font(surface().get_primary_font(), 18.0F);
        m_fill_color = theme.accent_color;
        configure_all_styles([&theme](Style& style) {
            style.color(theme.text_color)
                .background_color(theme.background_tertiary_color)
                .border(BorderEdge{1.0F, BorderType::Solid})
                .border_color(theme.border_color)
                .border_radius(5.0F)
                .padding({2.0F, 2.0F});
        });


    }

private:
    void paint_content(const PaintContext& context) override {
        const Rect rect = context.content_rect;
        const float progress = static_cast<float>(m_level.best_progress()) / 100.0F;
        if (progress > 0.0F) {
            context.painter.rect_filled({rect.min, {rect.min.x + rect.size().x * progress, rect.max.y}}, m_fill_color, 3.0F);
        }

        const std::string label = std::to_string(m_level.best_progress()) + "%";
        const ImVec2 size = ImGui::CalcTextSize(label.c_str());
        context.painter.text(
            {rect.min.x + (rect.size().x - size.x) * 0.5F, rect.min.y + (rect.size().y - size.y) * 0.5F},
            context.style.color().value, label
        );
    }

    const DashLevel& m_level;
    ui::Color m_fill_color;
};

LevelCard::LevelCard(const DashLevel& level) : ButtonWidget("", {percent(50), percent(50)}), m_level(level) {
    m_content = &add<Container>("level-card-content");
    m_content->set_size({grow(), grow()});
    m_content->set_input_mode(InputMode::None);
    m_content->set_content_alignment(Anchor::Center);
    m_content->set_spacing(10.0F);
    m_content->configure_all_styles([](Style& style) {
        style.background_color(rgba(0, 0, 0, 0));
        style.border(BorderEdge{0.0F, BorderType::None});
        style.padding({0.0F, 0.0F});
    });

    m_name = &m_content->add<TextWidget>(level.name());
    m_name->set_overflow(TextOverflow::Ellipsis);
    m_name->set_size({{fit(), px(0), percent(100)}, fit()});

    m_description = &m_content->add<TextWidget>(level.metadata().description);
    m_description->set_overflow(TextOverflow::Ellipsis);
    m_description->set_size({{fit(), px(0), percent(100)}, fit()});

    m_content->add<LevelProgressBar>(level);
}

void LevelCard::on_update(float) {
    m_name->set_text(m_level.name());
    m_description->set_text(m_level.metadata().description);
    m_description->set_visible(!m_level.metadata().description.empty());
}

void LevelCard::draw_children() {
    const auto content = content_rect(layout().visual_rect());
    const auto cursor = ImGui::GetCursorScreenPos();

    arrange_child(*m_content, m_content->layout().resolve_size(content.size()));
    ImGui::SetCursorScreenPos(content.min);
    m_content->draw_at_cursor();

    ImGui::SetCursorScreenPos(cursor);
}

void LevelCard::apply_theme_defaults(const Theme& theme) {
    ButtonWidget::apply_theme_defaults(theme);
    set_font(surface().get_primary_font(), 48);
    m_description->set_font(surface().get_primary_font(), 24);

    configure_all_styles([](Style& style) {
        style.background_color(rgb(36, 36, 36));
        style.padding({16.0F, 16.0F});
        style.border_radius(4.0F);
        style.border_thickness(2.0F);
    });
}
