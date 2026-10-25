#pragma once

#include "component.hpp"

#include <cstddef>
#include <vector>

// adapted from: https://www.raylib.com/examples/shapes/loader.html?name=shapes_simple_particles

struct Particle {
    bool update(float frametime);

    Vector2 position = {};
    Vector2 velocity = {};
    Vector2 acceleration = {};
    Color color = WHITE;
    float radius = 5.0F;
    float growth = 0.0F;
    float lifetime = 1.0F;
    float age = 0.0F;
};

class Particles : public Component {
public:
    explicit Particles(GameObject& object, std::size_t capacity = 3000);

    bool emit(Particle particle);
    void clear();
    void update(float frametime) override;
    void render(Rectangle = {}, const Camera2D& = {}, Vector2 = {}) const override;
    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const std::filesystem::path& directory) override;

    std::size_t size() const {
        return m_particles.size();
    }

    Particle prototype; // position is emitter-local
    float emission_rate = 0.0F;
    float opacity = 1.0F;

private:
    std::vector<Particle> m_particles;
    std::size_t m_capacity;
    double m_emission = 0.0;
};
