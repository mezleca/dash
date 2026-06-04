#pragma once

#include "../entity.hpp"

struct Spike : public Entity {
    explicit Spike(float ammount);

    void render() override;

private:
    float ammount = 0;
};
