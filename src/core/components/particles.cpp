#include "particles.hpp"
#include "rigid-body.hpp"
#include "core/object.hpp"

#include <algorithm>
#include <cmath>

bool Particle::update(float frametime) {
    age += frametime;
    if (age >= lifetime) {
        return false;
    }

    position.x += (velocity.x + 0.5F * acceleration.x * frametime) * frametime;
    position.y += (velocity.y + 0.5F * acceleration.y * frametime) * frametime;
    velocity.x += acceleration.x * frametime;
    velocity.y += acceleration.y * frametime;

    radius += growth * frametime;
    return radius > 0.0F;
}

Particles::Particles(GameObject& object, std::size_t capacity) : Component(object, ComponentId::Particles), m_capacity(capacity) {
    m_particles.reserve(capacity);
}

bool Particles::emit(Particle particle) {
    if (m_particles.size() == m_capacity || particle.radius <= 0.0F || particle.lifetime <= 0.0F) {
        return false;
    }

    particle.age = 0.0F;
    m_particles.push_back(particle);
    return true;
}

void Particles::clear() {
    m_particles.clear();
    m_emission = 0.0;
}

void Particles::update(float frametime) {
    if (frametime <= 0.0F) {
        return;
    }

    std::erase_if(m_particles, [frametime](Particle& particle) { return !particle.update(frametime); });

    if (emission_rate <= 0.0F || !std::isfinite(emission_rate)) return;

    m_emission += static_cast<double>(emission_rate) * frametime;
    const double pending = std::floor(m_emission);
    m_emission -= pending;
    const auto count = static_cast<std::size_t>(std::min(pending, static_cast<double>(m_capacity - m_particles.size())));
    if (count == 0) return;

    const auto* body = object().get_component<RigidBody>();
    const Rectangle area = body != nullptr ? body->get_world_bounding_box() : Rectangle{};
    for (std::size_t index = 0; index < count; ++index) {
        Particle particle = prototype;
        particle.position.x += area.x + area.width * static_cast<float>(GetRandomValue(0, 1000)) / 1000.0F;
        particle.position.y += area.y + area.height * static_cast<float>(GetRandomValue(0, 1000)) / 1000.0F;
        emit(particle);
    }
}

void Particles::render(Rectangle, const Camera2D&, Vector2) const {
    for (const auto& particle : m_particles) {
        Color color = particle.color;
        color.a = static_cast<unsigned char>(
            static_cast<float>(color.a) * (1.0F - particle.age / particle.lifetime) * std::clamp(opacity, 0.0F, 1.0F)
        );
        DrawCircleV(particle.position, particle.radius, color);
    }
}

void Particles::serialize(Serializer& serializer) const {
    serializer.field("emission_rate", emission_rate);
    serializer.field("offset", prototype.position);
    serializer.field("velocity", prototype.velocity);
    serializer.field("acceleration", prototype.acceleration);
    serializer.field("color", prototype.color);
    serializer.field("radius", prototype.radius);
    serializer.field("growth", prototype.growth);
    serializer.field("lifetime", prototype.lifetime);
}

void Particles::deserialize(Deserializer& deserializer, const std::filesystem::path&) {
    deserializer.field("emission_rate", emission_rate);
    deserializer.field("offset", prototype.position);
    deserializer.field("velocity", prototype.velocity);
    deserializer.field("acceleration", prototype.acceleration);
    deserializer.field("color", prototype.color);
    deserializer.field("radius", prototype.radius);
    deserializer.field("growth", prototype.growth);
    deserializer.field("lifetime", prototype.lifetime);
    clear();
}
