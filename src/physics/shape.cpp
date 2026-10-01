#include "shape.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

b2Polygon make_box_shape(float width, float height) {
    if (!std::isfinite(width) || !std::isfinite(height)) {
        throw std::invalid_argument("box size must be finite");
    }

    if (width <= 0.0f || height <= 0.0f) {
        std::cerr << "[shape] warning: box size must be positive\nusing at least 1 pixel\n";
    }

    width = std::max(width, 1.0f);
    height = std::max(height, 1.0f);

    const float half_width = width / (2.0f * PHYSICS_PIXELS_PER_METER);
    const float half_height = height / (2.0f * PHYSICS_PIXELS_PER_METER);

    // offset the box so body origin matches the sprite top left corner
    return b2MakeOffsetBox(half_width, half_height, {half_width, half_height}, b2Rot_identity);
}

void to_json(nlohmann::json& data, const b2Vec2& point) {
    data = Vector2{point.x * PHYSICS_PIXELS_PER_METER, point.y * PHYSICS_PIXELS_PER_METER};
}

void from_json(const nlohmann::json& data, b2Vec2& point) {
    const Vector2 pixels = data.get<Vector2>();

    if (!std::isfinite(pixels.x) || !std::isfinite(pixels.y)) {
        throw std::invalid_argument("shape coordinates must be finite");
    }

    point = {pixels.x / PHYSICS_PIXELS_PER_METER, pixels.y / PHYSICS_PIXELS_PER_METER};
}

void to_json(nlohmann::json& data, const b2Polygon& polygon) {
    data = {{"type", "polygon"}, {"radius", polygon.radius * PHYSICS_PIXELS_PER_METER}, {"vertices", nlohmann::json::array()}};

    for (int i = 0; i < polygon.count; ++i) {
        data["vertices"].push_back(polygon.vertices[i]);
    }
}

void from_json(const nlohmann::json& data, b2Polygon& polygon) {
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

    // box2d computes vertex order, normals, and centroid from the convex hull.
    const b2Hull hull = b2ComputeHull(points, count);
    if (hull.count != count || !b2ValidateHull(&hull)) {
        throw std::invalid_argument("polygon vertices must form a convex hull without duplicates");
    }

    polygon = b2MakePolygon(&hull, std::max(radius, 0.0f) / PHYSICS_PIXELS_PER_METER);
}

void to_json(nlohmann::json& data, const b2Circle& circle) {
    data = {{"type", "circle"}, {"center", circle.center}, {"radius", circle.radius * PHYSICS_PIXELS_PER_METER}};
}

void from_json(const nlohmann::json& data, b2Circle& circle) {
    const float radius = data.at("radius").get<float>();
    if (!std::isfinite(radius)) {
        throw std::invalid_argument("circle radius must be finite");
    }

    if (radius <= 0.0f) {
        std::cerr << "[shape] warning: nonpositive circle radius\nusing 1 pixel\n";
    }

    circle = {data.at("center").get<b2Vec2>(), std::max(radius, 1.0f) / PHYSICS_PIXELS_PER_METER};
}

void to_json(nlohmann::json& data, const b2Capsule& capsule) {
    data = {
        {"type", "capsule"},
        {"center1", capsule.center1},
        {"center2", capsule.center2},
        {"radius", capsule.radius * PHYSICS_PIXELS_PER_METER}
    };
}

void from_json(const nlohmann::json& data, b2Capsule& capsule) {
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
