#pragma once

#include "behaviour.hpp"
#include "game/level/camera.hpp"

class Entity;
class DashLevel;

class CameraFocus : public Behaviour {
public:
    CameraFocus(Entity& owner, DashLevel& level);
    BehaviourType type() const override {
        return BehaviourType::CameraFocus;
    }
    void update(float frametime) override;
    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;

    const CameraFocusSettings& settings() const {
        return m_settings;
    }
    void set_settings(const CameraFocusSettings& settings);

private:
    Entity& m_owner;
    DashLevel& m_level;
    CameraFocusSettings m_settings;
};
