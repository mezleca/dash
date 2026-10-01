#pragma once

#include <raylib.h>
#include <nlohmann/json.hpp>

inline static void to_json(nlohmann::json& j, const Vector2& v) {
    j = {{"x", v.x}, {"y", v.y}};
}

inline static void from_json(const nlohmann::json& j, Vector2& v) {
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
}
