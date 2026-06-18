#pragma once

#include <raylib.h>
#include <string>
#include <nlohmann/json.hpp>

inline static void to_json(nlohmann::json& j, const Vector2& v) {
    j = {{"x", v.x}, {"y", v.y}};
}

inline static void from_json(const nlohmann::json& j, Vector2& v) {
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
}

namespace json_helper {
    [[nodiscard]] inline std::string get_string(const nlohmann::json& j, std::string_view key) {
        auto it = j.find(key);

        if (it == j.end() || !it->is_string()) {
            return "";
        }

        return it->get<std::string>();
    }

    [[nodiscard]] inline bool get_bool(const nlohmann::json& j, std::string_view key) {
        auto it = j.find(key);

        if (it == j.end() || !it->is_boolean()) {
            return false;
        }

        return it->get<bool>();
    }

    [[nodiscard]] inline int get_int(const nlohmann::json& j, std::string_view key) {
        auto it = j.find(key);

        if (it == j.end() || !it->is_number_integer()) {
            return 0;
        }

        return it->get<int>();
    }

    [[nodiscard]] inline float get_float(const nlohmann::json& j, std::string_view key) {
        auto it = j.find(key);

        if (it == j.end() || !it->is_number_float()) {
            return 0.0f;
        }

        return it->get<float>();
    }

    [[nodiscard]] inline Vector2 get_vec2(const nlohmann::json& j, std::string_view key) {
        auto it = j.find(key);

        if (it == j.end()) {
            return {0.0f, 0.0f};
        }

        return it->get<Vector2>();
    }
} // namespace json_helper
