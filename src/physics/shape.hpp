#pragma once

#include "world.hpp"
#include "../utils/json.hpp"

b2Polygon make_box_shape(float width, float height);

void to_json(nlohmann::json& data, const b2Vec2& point);
void from_json(const nlohmann::json& data, b2Vec2& point);

void to_json(nlohmann::json& data, const b2Polygon& polygon);
void from_json(const nlohmann::json& data, b2Polygon& polygon);

void to_json(nlohmann::json& data, const b2Circle& circle);
void from_json(const nlohmann::json& data, b2Circle& circle);

void to_json(nlohmann::json& data, const b2Capsule& capsule);
void from_json(const nlohmann::json& data, b2Capsule& capsule);
