#pragma once

class Behaviour {
public:
    virtual ~Behaviour() = default;

    virtual void update(float frametime) = 0;

    void finished() {
        m_finished = true;
    }

    bool is_finished() const {
        return m_finished;
    }

private:
    bool m_finished = false;
};
