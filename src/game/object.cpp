#include "game.hpp"
#include "object.hpp"
#include "../physics/rigidbody.hpp"

#include <iostream>

GameObject::GameObject(ObjectType _type) : type(_type) {
    rb = std::make_unique<RigidBody>(this);

    velocity = {0, 0};
    position = {0, 0};

    visible = true;
}

GameObject::~GameObject() {
    if (texture.id) {
        UnloadTexture(texture);
    }

    game.remove_game_object(this);
}

void GameObject::load_texture(const char* location) {
    if (texture.id) UnloadTexture(texture);

    texture = LoadTexture(location);

    if (!IsTextureValid(texture)) {
        std::cout << "[object] failed to load texture from " << location << "\n";
        texture = {};
        texture_location.clear();
        return;
    }

    dimensions.x = static_cast<float>(texture.width);
    dimensions.y = static_cast<float>(texture.height);

    texture_location = location;
}
