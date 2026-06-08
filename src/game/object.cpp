#include "game.hpp"
#include "object.hpp"
#include "../physics/rigidbody.hpp"

GameObject::GameObject(ObjectType _type) : type(_type) {
    game.add_game_object(this);

    rb = new RigidBody(this);

    velocity = {0, 0};
    position = {0, 0};

    visible = true;
}

GameObject::~GameObject() {
    game.remove_game_object(this);
}

void GameObject::load_texture(const char* location) {
    if (texture.id) UnloadTexture(texture);

    texture = LoadTexture(location);

    dimensions.x = static_cast<float>(texture.width);
    dimensions.y = static_cast<float>(texture.height);
}
