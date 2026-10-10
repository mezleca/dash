#include "object.hpp"
#include "core/components/sprite.hpp"
#include "core/serialization/json.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

std::string GameObject::serialize_to_buffer(SerializationFormat format) const {
    if (format != SerializationFormat::Json) {
        throw std::invalid_argument("a concrete serialization format is required");
    }

    JsonSerializer serializer;
    serialize(serializer);
    return serializer.data().dump();
}

void GameObject::deserialize_from_buffer(
    std::string_view buffer, const std::filesystem::path& directory, SerializationFormat format
) {
    if (format != SerializationFormat::Json) {
        throw std::invalid_argument("a concrete serialization format is required");
    }

    const auto data = nlohmann::json::parse(buffer);
    JsonDeserializer deserializer(data);
    deserialize(deserializer, directory);
}

bool GameObject::save_to_file(const std::filesystem::path& file, SerializationFormat format) const {
    const auto selected = resolve_file_format(file, format);
    const std::string buffer = serialize_to_buffer(selected);

    std::ofstream output(file, std::ios::binary);
    if (!output) return false;

    output << buffer << '\n';
    output.close();
    return !output.fail();
}

void GameObject::load_from_file(const std::filesystem::path& file, SerializationFormat format) {
    const auto selected = resolve_file_format(file, format);
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open file: " + file.string());
    }

    const std::string buffer{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (input.bad()) {
        throw std::runtime_error("cannot read file: " + file.string());
    }

    deserialize_from_buffer(buffer, file.parent_path(), selected);
}

void GameObject::update(float frametime) {
    const size_t count = m_components.size();
    for (size_t i = 0; i < count; ++i) {
        m_components[i]->update(frametime);
    }
}

void GameObject::render(Rectangle bounds, const Camera2D& camera) const {
    if (!visible) return;

    const size_t count = m_components.size();
    for (size_t i = 0; i < count; ++i) {
        m_components[i]->render(bounds, camera);
    }
}

void GameObject::remove_component(Component& component) {
    std::erase_if(m_components, [&component](const auto& entry) { return entry.get() == &component; });
}

void GameObject::serialize(Serializer& serializer) const {
    if (!visible) {
        serializer.field("visible", false);
    }

    if (z_index != 0) {
        serializer.field("z_index", z_index);
    }

    serializer.begin_array("components");

    for (const auto& component : m_components) {
        const ComponentId id = component->id();
        if (id == ComponentId::None) {
            continue;
        }

        serializer.begin_element();
        serializer.field("component", static_cast<int>(id));
        component->serialize(serializer);
        serializer.end_element();
    }

    serializer.end_array();
}

void GameObject::deserialize(Deserializer& deserializer, const std::filesystem::path& directory) {
    visible = true;
    z_index = 0;
    deserializer.field("visible", visible);
    deserializer.field("z_index", z_index);

    const std::size_t count = deserializer.array_size("components");
    for (std::size_t index = 0; index < count; ++index) {
        deserializer.begin_element("components", index);

        int raw_id = 0;
        if (!deserializer.field("component", raw_id)) {
            throw std::invalid_argument("component id is missing");
        }

        const auto id = static_cast<ComponentId>(raw_id);
        auto component =
            std::find_if(m_components.begin(), m_components.end(), [id](const auto& entry) { return entry->id() == id; });

        if (component != m_components.end()) {
            (*component)->deserialize(deserializer, directory);
        } else if (id == ComponentId::Sprite) {
            add_component<Sprite>().deserialize(deserializer, directory);
        } else {
            std::cerr << "[object] warning: unknown component id " << raw_id << "\nskipping component\n";
        }

        deserializer.end_element();
    }
}
