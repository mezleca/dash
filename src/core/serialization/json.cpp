#include "json.hpp"
#include "core/physics/world.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

bool JsonDeserializer::supports_file(const std::filesystem::path& file) {
    return file.extension() == ".json";
}

[[maybe_unused]] static void to_json(nlohmann::json& data, const Vector2& value) {
    data = {{"x", value.x}, {"y", value.y}};
}

[[maybe_unused]] static void from_json(const nlohmann::json& data, Vector2& value) {
    data.at("x").get_to(value.x);
    data.at("y").get_to(value.y);
}

[[maybe_unused]] static void to_json(nlohmann::json& data, const b2Vec2& point) {
    data = Vector2{point.x * PHYSICS_PIXELS_PER_METER, point.y * PHYSICS_PIXELS_PER_METER};
}

[[maybe_unused]] static void from_json(const nlohmann::json& data, b2Vec2& point) {
    const Vector2 pixels = data.get<Vector2>();
    if (!std::isfinite(pixels.x) || !std::isfinite(pixels.y)) {
        throw std::invalid_argument("shape coordinates must be finite");
    }

    point = {pixels.x / PHYSICS_PIXELS_PER_METER, pixels.y / PHYSICS_PIXELS_PER_METER};
}

[[maybe_unused]] static void to_json(nlohmann::json& data, const b2Polygon& polygon) {
    data = {{"type", "polygon"}, {"radius", polygon.radius * PHYSICS_PIXELS_PER_METER}, {"vertices", nlohmann::json::array()}};
    for (int i = 0; i < polygon.count; ++i) {
        data["vertices"].push_back(polygon.vertices[i]);
    }
}

[[maybe_unused]] static void from_json(const nlohmann::json& data, b2Polygon& polygon) {
    const auto& vertices = data.at("vertices");
    const float radius = data.value("radius", 0.0f);
    if (!vertices.is_array() || vertices.size() < 3 || vertices.size() > B2_MAX_POLYGON_VERTICES || !std::isfinite(radius)) {
        throw std::invalid_argument("polygon requires 3 to 8 vertices and a finite radius");
    }

    if (radius < 0.0f) {
        std::cerr << "[shape] warning: negative polygon radius\nusing 0\n";
    }

    b2Vec2 points[B2_MAX_POLYGON_VERTICES] = {};
    const int count = static_cast<int>(vertices.size());
    for (int i = 0; i < count; ++i) {
        points[i] = vertices.at(static_cast<size_t>(i)).get<b2Vec2>();
    }

    // box2d builds normals and centroid from the validated convex hull.
    const b2Hull hull = b2ComputeHull(points, count);
    if (hull.count != count || !b2ValidateHull(&hull)) {
        throw std::invalid_argument("polygon vertices must form a convex hull without duplicates");
    }

    polygon = b2MakePolygon(&hull, std::max(radius, 0.0f) / PHYSICS_PIXELS_PER_METER);
}

[[maybe_unused]] static void to_json(nlohmann::json& data, const b2Circle& circle) {
    data = {{"type", "circle"}, {"center", circle.center}, {"radius", circle.radius * PHYSICS_PIXELS_PER_METER}};
}

[[maybe_unused]] static void from_json(const nlohmann::json& data, b2Circle& circle) {
    const float radius = data.at("radius").get<float>();
    if (!std::isfinite(radius)) {
        throw std::invalid_argument("circle radius must be finite");
    }

    if (radius <= 0.0f) {
        std::cerr << "[shape] warning: nonpositive circle radius\nusing 1 pixel\n";
    }

    circle = {data.at("center").get<b2Vec2>(), std::max(radius, 1.0f) / PHYSICS_PIXELS_PER_METER};
}

[[maybe_unused]] static void to_json(nlohmann::json& data, const b2Capsule& capsule) {
    data = {
        {"type", "capsule"},
        {"center1", capsule.center1},
        {"center2", capsule.center2},
        {"radius", capsule.radius * PHYSICS_PIXELS_PER_METER}
    };
}

[[maybe_unused]] static void from_json(const nlohmann::json& data, b2Capsule& capsule) {
    const float radius = data.at("radius").get<float>();
    if (!std::isfinite(radius)) {
        throw std::invalid_argument("capsule radius must be finite");
    }

    if (radius <= 0.0f) {
        std::cerr << "[shape] warning: nonpositive capsule radius\nusing 1 pixel\n";
    }

    capsule = {
        data.at("center1").get<b2Vec2>(), data.at("center2").get<b2Vec2>(), std::max(radius, 1.0f) / PHYSICS_PIXELS_PER_METER
    };
}

nlohmann::json& JsonSerializer::current() {
    return *m_stack.back();
}

void JsonSerializer::field(std::string_view name, bool value) {
    current()[std::string(name)] = value;
}

void JsonSerializer::field(std::string_view name, int value) {
    current()[std::string(name)] = value;
}

void JsonSerializer::field(std::string_view name, float value) {
    current()[std::string(name)] = value;
}

void JsonSerializer::field(std::string_view name, std::string_view value) {
    current()[std::string(name)] = std::string(value);
}

void JsonSerializer::field(std::string_view name, Vector2 value) {
    current()[std::string(name)] = value;
}

void JsonSerializer::field(std::string_view name, Color value) {
    current()[std::string(name)] = {value.r, value.g, value.b, value.a};
}

void JsonSerializer::field(std::string_view name, const PhysicsShape& value) {
    std::visit([this, name](const auto& shape) { current()[std::string(name)] = shape; }, value);
}

void JsonSerializer::begin_array(std::string_view name) {
    auto& array = current()[std::string(name)];
    array = nlohmann::json::array();
    m_stack.push_back(&array);
}

void JsonSerializer::begin_element() {
    auto& array = current();
    if (!array.is_array()) {
        throw std::logic_error("serializer is not inside an array");
    }

    array.push_back(nlohmann::json::object());
    m_stack.push_back(&array.back());
}

void JsonSerializer::end_array() {
    if (m_stack.size() <= 1 || !current().is_array()) {
        throw std::logic_error("serializer has no open array");
    }

    m_stack.pop_back();
}

void JsonSerializer::end_element() {
    if (m_stack.size() <= 1 || !current().is_object()) {
        throw std::logic_error("serializer has no open element");
    }

    m_stack.pop_back();
}

JsonDeserializer::JsonDeserializer(const nlohmann::json& data) : m_stack{&data} {}

const nlohmann::json& JsonDeserializer::current() const {
    return *m_stack.back();
}

const nlohmann::json* JsonDeserializer::find(std::string_view name) const {
    const auto entry = current().find(std::string(name));
    return entry == current().end() ? nullptr : &*entry;
}

template <class T>
static bool read_value(const nlohmann::json* entry, T& value) {
    if (entry == nullptr) {
        return false;
    }

    value = entry->get<T>();
    return true;
}

bool JsonDeserializer::field(std::string_view name, bool& value) const {
    return read_value(find(name), value);
}

bool JsonDeserializer::field(std::string_view name, int& value) const {
    return read_value(find(name), value);
}

bool JsonDeserializer::field(std::string_view name, float& value) const {
    return read_value(find(name), value);
}

bool JsonDeserializer::field(std::string_view name, std::string& value) const {
    return read_value(find(name), value);
}

bool JsonDeserializer::field(std::string_view name, Vector2& value) const {
    return read_value(find(name), value);
}

bool JsonDeserializer::field(std::string_view name, Color& value) const {
    const auto* entry = find(name);
    if (entry == nullptr) {
        return false;
    }

    if (entry->is_array()) {
        value = {
            entry->at(0).get<unsigned char>(), entry->at(1).get<unsigned char>(), entry->at(2).get<unsigned char>(),
            entry->at(3).get<unsigned char>()
        };
    } else {
        value = {
            entry->at("r").get<unsigned char>(), entry->at("g").get<unsigned char>(), entry->at("b").get<unsigned char>(),
            entry->at("a").get<unsigned char>()
        };
    }

    return true;
}

bool JsonDeserializer::field(std::string_view name, PhysicsShape& value) const {
    const auto* entry = find(name);
    if (entry == nullptr) {
        return false;
    }

    const std::string type = entry->at("type").get<std::string>();
    if (type == "polygon") {
        value = entry->get<b2Polygon>();
    } else if (type == "circle") {
        value = entry->get<b2Circle>();
    } else if (type == "capsule") {
        value = entry->get<b2Capsule>();
    } else {
        throw std::invalid_argument("unsupported shape type: " + type);
    }

    return true;
}

std::size_t JsonDeserializer::array_size(std::string_view name) const {
    const auto* entry = find(name);
    if (entry == nullptr) {
        return 0;
    }

    if (!entry->is_array()) {
        throw std::invalid_argument("expected an array: " + std::string(name));
    }

    return entry->size();
}

void JsonDeserializer::begin_element(std::string_view name, std::size_t index) {
    m_stack.push_back(&current().at(std::string(name)).at(index));
}

void JsonDeserializer::end_element() {
    if (m_stack.size() <= 1 || !current().is_object()) {
        throw std::logic_error("deserializer has no open element");
    }

    m_stack.pop_back();
}
