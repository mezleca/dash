#pragma once

class Serializer;
class Deserializer;

enum class BehaviourType {
    None,
    Pulse,
    CameraFocus,
    PlayerType,
};

class Behaviour {
public:
    virtual ~Behaviour() = default;
    virtual void update(float frametime) = 0;
    virtual BehaviourType type() const {
        return BehaviourType::None;
    }
    virtual void serialize(Serializer&) const {}
    virtual void deserialize(Deserializer&) {}
    virtual void reset() {
        m_finished = false;
    }
    static const char* type_to_str(BehaviourType type);
    const char* name() const {
        return type_to_str(type());
    }

    void finished() {
        m_finished = true;
    }

    bool is_finished() const {
        return m_finished;
    }

private:
    bool m_finished = false;
};
