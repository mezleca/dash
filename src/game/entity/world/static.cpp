#include "static.hpp"
#include "core/components/background-sprite.hpp"

StaticTexture::StaticTexture(World& world, bool fill_viewport) {
    add_component<RigidBody>(world, b2_staticBody, false, false);
    add_component<BackgroundSprite>(fill_viewport);
}
