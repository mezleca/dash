#pragma once

#include "behaviour.hpp"

class Sprite;

class Pulse : public Behaviour {
public:
    explicit Pulse(Sprite& sprite, float period = 1.2F, float minimum_opacity = 0.35F);
    void update(float frametime) override;
    void reset() override;
    BehaviourType type() const override {
        return BehaviourType::Pulse;
    }
    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;

    float period() const {
        return m_period;
    }
    void set_period(float period) {
        m_period = period;
    }
    float minimum_opacity() const {
        return m_minimum_opacity;
    }
    void set_minimum_opacity(float opacity) {
        m_minimum_opacity = opacity;
    }

private:
    Sprite& m_sprite;
    float m_period;
    float m_minimum_opacity;
    float m_phase = 0.0F;
};
