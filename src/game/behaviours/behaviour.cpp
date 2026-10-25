#include "behaviour.hpp"

const char* Behaviour::type_to_str(BehaviourType type) {
    switch (type) {
        case BehaviourType::Pulse:
            return "Pulse";
        case BehaviourType::CameraFocus:
            return "Camera focus";
        case BehaviourType::PlayerType:
            return "Player type";
        default:
            return "Behaviour";
    }
}
