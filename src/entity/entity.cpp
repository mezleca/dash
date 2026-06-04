#include "entity.hpp"
#include "../physics/rigidbody.hpp"

Entity::Entity() {
    rb = new RigidBody(this);

    velocity = {0, 0};
    position = {0, 0};
}
