#pragma once

#include "game/ui/modal.hpp"
#include <functional>

class Entity;
class Behaviour;
class DashDeleteButton;
class Dash;
class CameraFocus;

class BehaviourDialog : public Modal {
public:
    BehaviourDialog(Dash& game, std::function<void()> changed);
    void open(Entity& object, Behaviour* behaviour = nullptr);
    void close() override;
    void cancel_target_selection();

protected:
    void on_update(float frametime) override;
    void event(ui::UiEvent& event) override;

private:
    void rebuild();
    void select_target(CameraFocus& focus);

    Dash& m_game;
    std::function<void()> m_changed;
    Entity* m_object = nullptr;
    Behaviour* m_behaviour = nullptr;
    DashDeleteButton* m_remove = nullptr;
    bool m_rebuild = false;
};
