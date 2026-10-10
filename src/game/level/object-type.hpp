#pragma once

#include <cstdint>

enum class ObjectType : int32_t {
    NONE = -1,
    BOX,
    PLATFORM,
    SPIKE,
    END,
    STATIC_TEXTURE,
    TRIGGER
};
