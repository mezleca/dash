#include "entity.hpp"

const char* Entity::type_to_str(EntityType type) {
    switch (type) {
        case EntityType::BOX:
            return "box";
        case EntityType::PLATFORM:
            return "platform";
        case EntityType::SPIKE:
            return "spike";
        case EntityType::END:
            return "finish";
        case EntityType::STATIC_TEXTURE:
            return "static texture";
        case EntityType::TRIGGER:
            return "trigger";
        case EntityType::PLAYER:
            return "player";
        case EntityType::LEVEL:
            return "level";
        default:
            return "unknown";
    }
}
