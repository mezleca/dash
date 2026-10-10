#include "serialization.hpp"
#include "json.hpp"

#include <stdexcept>

SerializationFormat resolve_file_format(const std::filesystem::path& file, SerializationFormat format) {
    if (format != SerializationFormat::Auto) return format;
    if (JsonDeserializer::supports_file(file)) return SerializationFormat::Json;

    throw std::invalid_argument("unsupported file format: " + file.extension().string());
}

bool Deserializer::supports_file(const std::filesystem::path& file) {
    return JsonDeserializer::supports_file(file);
}
