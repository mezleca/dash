#pragma once

#include "serialization.hpp"

#include <nlohmann/json.hpp>
#include <vector>

class JsonSerializer : public Serializer {
public:
    JsonSerializer() = default;

    void field(std::string_view name, bool value) override;
    void field(std::string_view name, int value) override;
    void field(std::string_view name, float value) override;
    void field(std::string_view name, std::string_view value) override;
    void field(std::string_view name, Vector2 value) override;
    void field(std::string_view name, Color value) override;
    void field(std::string_view name, const PhysicsShape& value) override;

    void begin_array(std::string_view name) override;
    void end_array() override;
    void begin_element() override;
    void end_element() override;

    const nlohmann::json& data() const {
        return m_data;
    }

private:
    nlohmann::json& current();

    nlohmann::json m_data = nlohmann::json::object();
    std::vector<nlohmann::json*> m_stack = {&m_data};
};

class JsonDeserializer : public Deserializer {
public:
    explicit JsonDeserializer(const nlohmann::json& data);

    static bool supports_file(const std::filesystem::path& file);

    bool field(std::string_view name, bool& value) const override;
    bool field(std::string_view name, int& value) const override;
    bool field(std::string_view name, float& value) const override;
    bool field(std::string_view name, std::string& value) const override;
    bool field(std::string_view name, Vector2& value) const override;
    bool field(std::string_view name, Color& value) const override;
    bool field(std::string_view name, PhysicsShape& value) const override;

    std::size_t array_size(std::string_view name) const override;
    void begin_element(std::string_view name, std::size_t index) override;
    void end_element() override;

private:
    const nlohmann::json& current() const;
    const nlohmann::json* find(std::string_view name) const;

    std::vector<const nlohmann::json*> m_stack;
};
