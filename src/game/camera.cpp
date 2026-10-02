#include "camera.hpp"
#include "../utils/math.hpp"

void GameCamera::set_target(Vector2 target, Vector2 smoothing) {
    m_transform.target = {
        d_math::lerp(m_transform.target.x, target.x, smoothing.x), d_math::lerp(m_transform.target.y, target.y, smoothing.y)
    };
}

void GameCamera::center_viewport(Vector2 size) {
    m_transform.offset = {size.x / 2.0f, size.y / 2.0f};
}
