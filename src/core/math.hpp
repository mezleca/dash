#pragma once

#include <cmath>

namespace d_easing {
    inline float none(float t) {
        return t;
    }

    inline float out(float t) {
        return 1.0f - std::pow(1.0f - t, 2.0f);
    }

    inline float in(float t) {
        return t * t;
    }

    inline float in_out(float t) {
        if (t < 0.5f) {
            return 2.0f * t * t;
        }
        return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }

    inline float out_cubic(float t) {
        return 1.0f - std::pow(1.0f - t, 3.0f);
    }

    inline float in_cubic(float t) {
        return t * t * t;
    }

    inline float out_quad(float t) {
        return 1.0f - std::pow(1.0f - t, 2.0f);
    }

    inline float in_quad(float t) {
        return t * t;
    }

    inline float out_quint(float t) {
        return 1.0f - std::pow(1.0f - t, 5.0f);
    }

    inline float in_quint(float t) {
        return t * t * t * t * t;
    }
} // namespace d_easing

namespace d_math {
    inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
} // namespace d_math
