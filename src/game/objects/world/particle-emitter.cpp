#include "particle-emitter.hpp"

#include "core/components/particles.hpp"
#include "core/components/rigid-body.hpp"
#include "invisible-sprite.hpp"

ParticleEmitter::ParticleEmitter(World& world, DashLevel& level) : Entity(EntityType::PARTICLES) {
    add_component<RigidBody>(world, b2_staticBody, true, false);
    add_component<InvisibleSprite>(level, "particles", PURPLE);

    auto& particles = add_component<Particles>();
    particles.emission_rate = 30.0F;
    particles.prototype.velocity = {0.0F, -60.0F};
    particles.prototype.growth = 5.0F;
    particles.prototype.lifetime = 1.5F;
}
