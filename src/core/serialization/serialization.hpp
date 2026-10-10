#pragma once

#include <box2d/box2d.h>
#include <raylib.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <variant>

using PhysicsShape = std::variant<b2Polygon, b2Circle, b2Capsule>;

enum class SerializationFormat {
    Auto,
    Json
};

SerializationFormat resolve_file_format(const std::filesystem::path& file, SerializationFormat format);

class Serializer {
public:
    virtual ~Serializer() = default;

    virtual void field(std::string_view name, bool value) = 0;
    virtual void field(std::string_view name, int value) = 0;
    virtual void field(std::string_view name, float value) = 0;
    virtual void field(std::string_view name, std::string_view value) = 0;
    virtual void field(std::string_view name, Vector2 value) = 0;
    virtual void field(std::string_view name, Color value) = 0;
    virtual void field(std::string_view name, const PhysicsShape& value) = 0;

    virtual void begin_array(std::string_view name) = 0;
    virtual void end_array() = 0;
    virtual void begin_element() = 0;
    virtual void end_element() = 0;
};

class Deserializer {
public:
    virtual ~Deserializer() = default;

    static bool supports_file(const std::filesystem::path& file);

    virtual bool field(std::string_view name, bool& value) const = 0;
    virtual bool field(std::string_view name, int& value) const = 0;
    virtual bool field(std::string_view name, float& value) const = 0;
    virtual bool field(std::string_view name, std::string& value) const = 0;
    virtual bool field(std::string_view name, Vector2& value) const = 0;
    virtual bool field(std::string_view name, Color& value) const = 0;
    virtual bool field(std::string_view name, PhysicsShape& value) const = 0;

    virtual std::size_t array_size(std::string_view name) const = 0;
    virtual void begin_element(std::string_view name, std::size_t index) = 0;
    virtual void end_element() = 0;
};
