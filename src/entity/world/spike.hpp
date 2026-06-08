#pragma once

#include "../../game/object.hpp"

struct Spike : public GameObject {
    explicit Spike(float ammount);

    void render() override;

  private:
    float ammount = 0;
};
