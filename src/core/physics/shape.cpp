#include "shape.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

b2Polygon make_box_shape(float width, float height) {
    if (!std::isfinite(width) || !std::isfinite(height)) {
        throw std::invalid_argument("box size must be finite");
    }

    if (width <= 0.0f || height <= 0.0f) {
        std::cerr << "[shape] warning: box size must be positive\nusing at least 1 pixel\n";
    }

    width = std::max(width, 1.0f);
    height = std::max(height, 1.0f);

    const float half_width = width / (2.0f * PHYSICS_PIXELS_PER_METER);
    const float half_height = height / (2.0f * PHYSICS_PIXELS_PER_METER);

    // offset the box so body origin matches the sprite top left corner
    return b2MakeOffsetBox(half_width, half_height, {half_width, half_height}, b2Rot_identity);
}
