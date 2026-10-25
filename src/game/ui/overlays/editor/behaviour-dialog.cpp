#include "behaviour-dialog.hpp"

#include "game/objects/entities/entity.hpp"
#include "core/components/sprite.hpp"
#include "game/ui/controls.hpp"
#include "game/ui/widgets/behaviour-editor.hpp"
#include "game/editor/editor.hpp"
#include "game/dash.hpp"
#include "game/behaviours/camera-focus.hpp"

#include <imgui-ui/surface.hpp>
#include <imgui-ui/layout/container.hpp>
#include <imgui-ui/widgets/text.hpp>
#include <utility>

using namespace ui;

BehaviourDialog::BehaviourDialog(Dash& game, std::function<void()> changed)
    : Modal("behaviour-dialog"), m_game(game), m_changed(std::move(changed)) {
    m_remove = &m_actions->add<DashDeleteButton>("remove behaviour");
    m_remove->on_click([this] {
        m_object->remove_behaviour(*m_behaviour);
        m_changed();
        close();
    });
}

void BehaviourDialog::open(Entity& object, Behaviour* behaviour) {
    m_object = &object;
    m_behaviour = behaviour;
    m_rebuild = true;
    m_body->set_enabled(false);
    Modal::open();
}

void BehaviourDialog::close() {
    m_object = nullptr;
    m_behaviour = nullptr;
    m_rebuild = false;
    if (state() == ModalState::Minimized) cancel_target_selection();
    Modal::close();
}

void BehaviourDialog::select_target(CameraFocus& focus) {
    auto* editor = m_game.editor();
    if (editor == nullptr) return;
    set_title("Select a target");
    minimize();
    editor->begin_pick_target([this, &focus](Entity* target) {
        if (m_behaviour == nullptr) return;
        if (target != nullptr) {
            auto settings = focus.settings();
            settings.m_object_id = target->id();
            focus.set_settings(settings);
            m_changed();
        }
        m_body->set_enabled(false);
        m_rebuild = true;
        restore();
    });
}

void BehaviourDialog::cancel_target_selection() {
    if (auto* editor = m_game.editor()) editor->handle_event({EditorEventType::Cancel});
}

void BehaviourDialog::on_update(float) {
    if (visible() && m_rebuild) rebuild();
}

void BehaviourDialog::rebuild() {
    m_rebuild = false;
    m_body->clear();
    m_body->set_enabled(true);
    m_body->set_scrollable(m_behaviour != nullptr);
    m_remove->set_visible(m_behaviour != nullptr);
    set_title(m_behaviour != nullptr ? m_behaviour->name() : "Add behaviour");
    if (m_behaviour != nullptr) {
        m_body->add<BehaviourEditor>(
            *m_object, *m_behaviour, m_changed,
            [this] {
                m_changed();
                m_body->set_enabled(false);
                m_rebuild = true;
            },
            [this](CameraFocus& focus) { select_target(focus); }
        );
        return;
    }

    auto& list = m_body->add<Container>("behaviour-types", StackDirection::Horizontal);
    list.set_wrap(StackWrap::Wrap);
    list.set_spacing(8.0F);
    list.set_scrollable(true);

    const BehaviourType types[] = {BehaviourType::Pulse, BehaviourType::CameraFocus, BehaviourType::PlayerType};
    for (const auto type : types) {
        if (type == BehaviourType::Pulse && m_object->get_component<Sprite>() == nullptr) continue;
        if ((type == BehaviourType::CameraFocus || type == BehaviourType::PlayerType) && m_object->level() == nullptr) continue;
        auto& button = list.add<DashButton>(Behaviour::type_to_str(type), LayoutSize{px(200), px(64)});
        button.set_font(surface().get_primary_font(), 18.0F);
        button.on_click([this, type] {
            m_object->add_behaviour(type);
            m_changed();
            close();
        });
    }
}

void BehaviourDialog::event(UiEvent& input) {
    if (state() == ModalState::Minimized && input.type == EventType::KeyDown && input.key == Key::Escape) {
        cancel_target_selection();
        input.stop_propagation();
        input.block_native_input();
        return;
    }
    Modal::event(input);
}
